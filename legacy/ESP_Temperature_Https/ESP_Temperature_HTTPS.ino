#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char *ssid = "";
const char *password = "";
const char *host = "monitor.e-ander.pl";
const int httpsPort = 443;  

// OneWire set
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//Devices Set
DeviceAddress sens1 = { 0x28, 0xB9, 0xCF, 0x08, 0x4C, 0x20, 0x01, 0xCB };
DeviceAddress sens2 = { 0x28, 0xD3, 0xE0, 0x32, 0x46, 0x20, 0x01, 0x69 };

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "ES 28 13 0D 40 B1 00 ED BC 3F 08 55 00 AA B4 1B 28 9F 71 11";

//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(115200);
  sensors.begin();
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //Only Station No AP, This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
      //Declare object of class WiFiClient

  Serial.println(host);

// Start reading temperatures
   sensors.requestTemperatures(); // Send the command to get temperatures
  String temp1 = String(sensors.getTempC(sens1));
  String temp2 = String(sensors.getTempC(sens2));

  
    //postlaravel("room_cable",temp1);
    //postlaravel("out_room_cable",temp2);
  
 
  Serial.print("Sensor 1(*C): ");
  Serial.print(sensors.getTempC(sens1)); 
  Serial.println();
  
  Serial.print("Sensor 2(*C): ");
  Serial.print(sensors.getTempC(sens2)); 
  Serial.println();
  
  
  Serial.println();
 
  
  

 

    postlaravel("Piec",temp1);
    postlaravel("Piwnica",temp2);
    
  for( int i = 1; i <=4; i++){
    int time_remaining = 4 - i;
    Serial.print(time_remaining);
    Serial.println(" minutes remaining");
  delay(60000);
  
  }
}

void postlaravel (String device_id, String temperature_data){
  WiFiClientSecure httpsClient;
 Serial.printf("Using fingerprint '%s'\n", fingerprint);
    httpsClient.setInsecure(); 
  httpsClient.setTimeout(15000); // 15 Seconds
  delay(1000);
  
  Serial.print("HTTPS Connecting");
  int r=0; //retry counter
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)){
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r==30) {
    Serial.println("Connection failed");
  }
  else {
    Serial.println("Connected to web");
  }
  
  String getData, Link;
  
  //POST Data
  Link = "/api/temperatures/?device=" + device_id + "&temperature=" + temperature_data;

  Serial.print("requesting URL: ");
  Serial.println(host);
 
  httpsClient.print(String("POST ") + Link + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: none"+ "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
                  
  while (httpsClient.connected()) {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  Serial.println("reply was:");
  Serial.println("==========");
  String line;
  while(httpsClient.available()){        
    line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("==========");
  Serial.println("closing connection");

}
