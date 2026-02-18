# ESP_Temperature_HTTPS (legacy)

**Status:** Legacy (stable & proven)  
**Purpose:** DS18B20 temperature reporting firmware for ESP8266.  
**History:** Last device is running for ~6 years (basement device). A fork/variant was also used with **4 sensors**.

This sketch was designed long before `monitor_v2` and posts temperatures to an **older backend API**.

---

## What it does

- connects to WiFi (credentials were historically hardcoded; now must be provided locally)
- reads DS18B20 sensors using OneWire
- distinguishes sensors using hardcoded `DeviceAddress` values
- sends readings to a legacy HTTPS endpoint

---

## Dependencies

- `OneWire`
- `DallasTemperature`
- ESP8266 built-ins: `ESP8266WiFi`, `WiFiClientSecure`, `ESP8266HTTPClient`
- (optional) `ArduinoJson` if you later migrate to JSON payloads (recommended for `monitor_v2`)

---

## Legacy backend behavior

This sketch targets an old endpoint pattern similar to:
- `POST /api/temperatures/?device=<id>&temperature=<value>`

It is **not compatible** with the `monitor_v2` ingest API without refactoring.

---

## Known limitations

- Sensor identification uses hardcoded `DeviceAddress` values.
  - Works, but is painful when swapping sensors or flashing multiple devices.
- TLS handling is simplified (often `setInsecure()` style).
  - Acceptable for internal networks, but document if used on public internet.

---

## Migration recommendations (to monitor_v2)

For the new firmware under `devices/`:

1) Replace legacy endpoint calls with:
- `POST /api/ingest/{uuid}`
- header `X-Device-Secret: <secret>`
- JSON payload: `{ "temp": 21.5, "temp2": 20.9, ... }`

2) Remove hardcoded DS18B20 addresses:
- store address→channel mapping (`temp`, `temp2`, `temp3`, `temp4`) in EEPROM/NVS
- provide a simple “wizard” (serial or captive portal) to assign sensors on first boot

3) Keep the “proven” sampling + stability approach:
- stable intervals (e.g. every 60s / 300s)
- watchdog-safe loop
- basic health metadata (`rssi`, `fw`, `uptime_s`)

---

## Where this belongs

Keep this sketch in:
- `legacy/ESP_Temperature_HTTPS/`

New work should happen in:
- `devices/temp_x2/`
- `devices/temp_x4/`
with shared modules in `core/`.
