#include<stdlib.h>

#include <DHT.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define POWERED false // true

#if 1  // POWERED
#include <ArduinoOTA.h>
#endif

#include "secrets.h"

#if !POWERED
// Set your Static IP address
IPAddress local_IP(192, 168, 0, 124);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional
#endif

const char* host = "192.168.0.9";
//const char* host = "192.168.0.192";

#include <Wire.h>

ADC_MODE(ADC_VCC);

volatile bool load_active;

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println("\nBooting");
#if POWERED
  Serial.println("Powered");
#else
  Serial.println("Not Powered");
#endif

#if !POWERED
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
#if !POWERED
    delay(50);
    ESP.deepSleep(1 * 60 * 1000000);
#else
    delay(10000);
    ESP.restart();
#endif
  }
#endif
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
#if !POWERED
    delay(50);
    ESP.deepSleep(1 * 60 * 1000000);
#else
    delay(10000);
    ESP.restart();
#endif
  }

#if 1 // POWERED
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    load_active = true;
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    load_active = false;
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, progress % 10 == 0);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
#endif
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //I2C stuff
  Wire.begin(0, 2);
  //Wire.setClock(10000);
}


WiFiUDP Udp;

void loop() {

  Wire.requestFrom(0x1, 4);  // 4 bytes from device 1 (AtTiny)
  
  Serial.println("Read I2C");
  unsigned i2c = 0;
  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    Serial.println((int)c); // print the byte
    i2c = (i2c << 8) | c;
  }
  Serial.print(i2c);
  Serial.println(": Done");
  
  Udp.beginPacket(host, 41234);

  static char msg[1024];

  sprintf(msg, "%d 1 %d 3 %d", ESP.getChipId(), ESP.getVcc(), i2c);
  Serial.println(msg);
  Udp.write(msg, strlen(msg));
  Udp.endPacket();
  
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, HIGH);
  delay(50);

  if (!i2c) {  // Reset by hand
    Serial.println("Wait 20s for firmware OTA upload");
    pinMode(LED_BUILTIN, OUTPUT);
    auto endt = millis() + 20000;
    while (millis() < endt) {
      if (!load_active)
        digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      ArduinoOTA.handle();
      if (!load_active)
        digitalWrite(LED_BUILTIN, LOW);
      delay(500);
    }
  }

  while (ESP.getVcc() > 3800)  // Use power to lower voltage in solar charged super capacitor
    ;

#if !POWERED
  ESP.deepSleep(0);
#else
#if LED_BUILTIN != 1
  unsigned long start= millis();
  const int min= 1000, max= 1022, step= 2;
  int count= min;
  int dir= step;
  while (millis()-start < 60000) {
    if (count >= max)
      dir= -step;
    else if (count <= min)
      dir= step;
    count+= dir;
    analogWrite(LED_BUILTIN, count);
    ArduinoOTA.handle();
    delay(200);
  }
#else
  delay(500);
  ArduinoOTA.handle();
  delay(500);
#endif
#endif
}
