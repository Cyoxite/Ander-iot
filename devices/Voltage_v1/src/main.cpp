\
#include <Arduino.h>
#include <Wire.h>

#if defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Adafruit_INA219.h>

// Local-only secrets file (DO NOT COMMIT)
#include "secrets.h"

static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;
static const uint32_t HTTP_TIMEOUT_MS         = 7000;
static const uint32_t REPORT_INTERVAL_MS      = 30000;

Adafruit_INA219 ina219;

static String firmwareId()
{
  return String(FW_NAME) + "@" + String(FW_VERSION);
}

static bool connectWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  const uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
    yield();
  }

  return (WiFi.status() == WL_CONNECTED);
}

static bool postVoltage(float voltageV, int rssi)
{
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  WiFiClient client;
#if defined(ARDUINO_ARCH_ESP8266)
  HTTPClient http;
#else
  HTTPClient http;
#endif

  String url = String(MONITOR_BASE_URL) + "/api/ingest/" + String(DEVICE_UUID);

  if (!http.begin(client, url)) {
    return false;
  }

  http.setTimeout(HTTP_TIMEOUT_MS);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-Secret", DEVICE_SECRET);

  StaticJsonDocument<256> doc;
  doc["voltage"] = voltageV;
  doc["fw"]      = firmwareId();
  doc["rssi"]    = rssi;

  String payload;
  serializeJson(doc, payload);

  int code = http.POST(payload);
  String resp = http.getString();
  http.end();

  Serial.printf("[HTTP] POST %s -> %d\n", url.c_str(), code);
  if (resp.length() > 0) {
    Serial.printf("[HTTP] Response: %s\n", resp.c_str());
  }

  return (code >= 200 && code < 300);
}

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.printf("Boot: %s\n", firmwareId().c_str());

  // I2C init
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!ina219.begin()) {
    Serial.println("INA219 not found. Check wiring (SDA/SCL/VCC/GND).");
  } else {
    Serial.println("INA219 OK.");
  }

  Serial.printf("Connecting WiFi: %s\n", WIFI_SSID);
  if (connectWiFi()) {
    Serial.printf("WiFi OK. IP: %s, RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    Serial.println("WiFi connect failed (timeout). Will retry in loop.");
  }
}

void loop()
{
  static uint32_t last = 0;

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  const uint32_t now = millis();
  if (now - last >= REPORT_INTERVAL_MS) {
    last = now;

    // Read INA219
    const float busV     = ina219.getBusVoltage_V();        // V at VIN- relative to GND
    const float shuntmV  = ina219.getShuntVoltage_mV();     // mV across shunt (VIN+ - VIN-)
    const float loadV    = busV + (shuntmV / 1000.0f);      // approximate supply at VIN+
    const int   rssi     = (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;

    Serial.printf("busV=%.3fV shunt=%.3fmV loadV=%.3fV rssi=%d\n", busV, shuntmV, loadV, rssi);

    // Send to backend
    const bool ok = postVoltage(loadV, rssi);
    Serial.printf("Ingest: %s\n", ok ? "OK" : "FAIL");
  }

  delay(10);
}
