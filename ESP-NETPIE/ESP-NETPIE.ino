/*
   Royal Project Weather Station

   nodeMCU     Sensor
   ===================
   D0       |   Wake Up
   D1       |   SDA
   D2       |   SCL
   D3       |   Wind
   D4       |   DHT
   D5       |   SD-CLK
   D6       |   SD-MISO
   D7       |   SD-MOSI
   D8       |   SD-CS
   SD2      |   Relay
   SD3      |   Rain drop



   ampere :
   http://192.168.12.214:1880/

*/


#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_ADS1015.h>
#include <BH1750.h>
#include "RTClib.h"
#include "DHT.h"


const char* ssid     = "ampere";
const char* password = "espertap";

#define APPID   "RoyalProjectStation"
#define KEY     "knIlIaf3SoxmZoc"
#define SECRET  "PBavMWrfGyrsdFgQRiZZXiM6h"

//#define APPID   "rpfew30021001"
//#define KEY     "FmB2CDjrdubJAWO"
//#define SECRET  "KASG7Ut1TbVXqJLt56nBEzARQ"

#define ALIAS   "royal1"

// init SD card
File myFile;

// init RTC
RTC_DS1307 rtc;


// init BH1750
BH1750 lightMeter;


// init ads1115
Adafruit_ADS1115 ads(0x48);
#define Offset 0
#define WindSensorPin D3


//init dht
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// init variable
int VaneValue;// raw analog value from wind vane
int Direction;// translated 0 - 360 direction
int CalDirection;// converted value with offset applied
int LastValue;
volatile unsigned long Rotations;
volatile unsigned long ContactBounceTime;
float WindSpeed;
unsigned long preMillis = 0;
const long intervalSpeed = 10000;  // send data every 10 second
String windDirection = "";

// init rain gauge
const byte RainDropPin = 9;
const int intervalRain = 500;
volatile unsigned long tiptime = millis();
unsigned long tipcount;
double rainrate;

WiFiClient client;

int timer = 0;
MicroGear microgear(client);

/* If a new message arrives, do this */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}

/* When a microgear is connected, do this */
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  /* Set the alias of this microgear ALIAS */
  microgear.setAlias(ALIAS);
}


void setup() {
  /* Add Event listeners */
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);


  Serial.begin(115200);
  delay(2000);
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }


  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);


  rtc.begin();
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


  Wire.begin(D1, D2);

  lightMeter.begin();
  dht.begin();
  ads.begin();
  LastValue = 1;

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

  pinMode(RainDropPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(RainDropPin), countRain, FALLING);

  firstSave();
  delay(2000); // delay for write to sd card
}

void loop() {
  /* To check if the microgear is still connected */
  if (microgear.connected()) {
    microgear.loop();
    unsigned long curMillis = millis();
    if (curMillis - preMillis >= intervalSpeed) {
      preMillis = curMillis;

      Rotations = 0;
      sei(); delay(1000); cli();

      WindSpeed = Rotations * 0.75;

      // call wind speed and direction
      VaneValue = map(ads.readADC_SingleEnded(3), 0, 17550, 0, 1023);
      Direction = map(VaneValue, 0, 1023, 0, 360);
      CalDirection = Direction + Offset;

      if (CalDirection > 360) {
        CalDirection = CalDirection - 360;
      } else if (CalDirection < 0) {
        CalDirection = CalDirection + 360;
      }


      // clear rain drop
      rainrate = 0;

      // read direction
      getHeading(CalDirection);

      // read temperature
      float t = dht.readTemperature();
      float h = dht.readHumidity();


      // read voltage
      float voltage = map(ads.readADC_SingleEnded(1), 0, 25000, 0, 1023) * (5.0 / 1023.0);


      // read light
      uint16_t lux = lightMeter.readLightLevel();


      // update dateTime
      DateTime now = rtc.now();
      String _date = String(now.day()) + "/" + String(now.month()) + "/" + String(now.year());
      String _time = String(now.hour()) + ":" + String(now.minute());



      // send to netpie
      char topic_data[MAXTOPICSIZE];
      char topic_date[MAXTOPICSIZE];
      char topic_time[MAXTOPICSIZE];
      char topic_temp[MAXTOPICSIZE];
      char topic_humid[MAXTOPICSIZE];
      char topic_rain[MAXTOPICSIZE];
      char topic_wind[MAXTOPICSIZE];
      char topic_direc[MAXTOPICSIZE];
      char topic_light[MAXTOPICSIZE];
      char topic_batt[MAXTOPICSIZE];

      sprintf(topic_data, "/gearname/%s/data", ALIAS);
      sprintf(topic_date, "/gearname/%s/date", ALIAS);
      sprintf(topic_time, "/gearname/%s/time", ALIAS);
      sprintf(topic_temp, "/gearname/%s/temp", ALIAS);
      sprintf(topic_humid, "/gearname/%s/humid", ALIAS);
      sprintf(topic_rain, "/gearname/%s/rain", ALIAS);
      sprintf(topic_wind, "/gearname/%s/wind", ALIAS);
      sprintf(topic_direc, "/gearname/%s/direc", ALIAS);
      sprintf(topic_light, "/gearname/%s/light", ALIAS);
      sprintf(topic_batt, "/gearname/%s/batt", ALIAS);

      String allData = String(_date) + "," + String(_time) + "," + String(t) + "," + String(h) + "," + String(rainrate)
                       + "," + String(WindSpeed) + "," + String(windDirection) + "," + String(lux) + "," + String(voltage);

      microgear.publish(topic_data, String(allData), true);
      microgear.publish(topic_date, String(_date), true);
      microgear.publish(topic_time, String(_time), true);
      microgear.publish(topic_temp, String(t), true);
      microgear.publish(topic_humid, String(h), true);
      microgear.publish(topic_rain, String(rainrate), true);
      microgear.publish(topic_wind, String(WindSpeed), true);
      microgear.publish(topic_direc, String(windDirection), true);
      microgear.publish(topic_light, String(lux), true);
      microgear.publish(topic_batt, String(voltage), true);

      dataLog(String(t), String(h), String(rainrate), String(WindSpeed), String(windDirection), String(lux), String(voltage));
      delay(2000); // delay for write to sd card

      //      microgear.chat("rjSensor1/date", _date);
      //      microgear.chat("rjSensor1/time", _time);
      //      if (!isnan(t)) microgear.chat("rjSensor1/temp", String(t));
      //      if (!isnan(h)) microgear.chat("rjSensor1/humid", String(h));
      //      microgear.chat("rjSensor1/rain", String(rainrate));
      //      microgear.chat("rjSensor1/wind", String(WindSpeed));
      //      microgear.chat("rjSensor1/direc", String(windDirection));
      //      microgear.chat("rjSensor1/light", String(lux));
      //      microgear.chat("rjSensor1/batt", String(voltage));


      Serial.print("Date: ");
      Serial.print(_date);
      Serial.print("\t Time: ");
      Serial.println(_time);

      Serial.print("Rain rate: ");
      Serial.print(rainrate);
      Serial.println(" mm/hr");

      Serial.print("Temp : ");
      Serial.print(t);
      Serial.print("\tHumid : ");
      Serial.println(h);

      Serial.print("Wind Speed : ");
      Serial.print(WindSpeed);
      Serial.print("\tDirection : ");
      Serial.println(windDirection);

      Serial.print("lightMeter : ");
      Serial.print(lux);
      Serial.print("\tbatt : ");
      Serial.print(voltage);

      Serial.println(" ");
      Serial.println(" ");
    }
  }
  else {
    Serial.println("connection lost, reconnect...");
    if (timer >= 5000) {
      microgear.connect(APPID);
      timer = 0;
    }
    else timer += 100;
  }
  delay(100);
}
