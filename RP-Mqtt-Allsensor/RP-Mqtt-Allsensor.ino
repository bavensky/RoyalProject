/*
 * Royal Project Weather Station
 * 
 * nodeMCU     Sensor
 * ===================
 * D0       |   Wake Up
 * D1       |   SDA
 * D2       |   SCL
 * D3       |   Wind
 * D4       |   DHT
 * D5       |   SD-CLK
 * D6       |   SD-MISO
 * D7       |   SD-MOSI
 * D8       |   SD-CS
 * SD2      |   Relay
 * SD3      |   Rain drop
 * 
 * 
 * 
 * ampere :
 * http://192.168.12.214:1880/
 * 
 */



#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <MqttConnector.h>
#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <Adafruit_ADS1015.h>

// ads1115 initialization
Adafruit_ADS1115 ads(0x48);

#define Offset 0
#define DHT_PIN D4
#define WindSensorPin  9
#define RainDropPin    10

#include "init_mqtt.h"
#include "_publish.h"
#include "_receive.h"
#include "_config.h"


MqttConnector *mqtt;
DHT dht(DHT_PIN, DHT22);


int relayPin            = 9;
int relayPinState       = HIGH;


char myName[40];



void init_hardware()
{
  // serial port initialization
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  pinMode(relayPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DHT_PIN, INPUT_PULLUP);
  digitalWrite(relayPin, relayPinState);

  dht.begin();


  Wire.begin(D1, D2);
  ads.begin();
  LastValue = 1;

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

  pinMode(RainDropPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(RainDropPin), countRain, FALLING);
}

void init_wifi() {
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(50);
  const char* ssid =  WIFI_SSID.c_str();
  const char* pass =  WIFI_PASSWORD.c_str();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", ssid, pass);
    delay(300);
  }
  Serial.println("WiFi Connected.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup()
{
  init_hardware();
  init_wifi();
  init_mqtt();
}

void loop()
{
  mqtt->loop();
}
