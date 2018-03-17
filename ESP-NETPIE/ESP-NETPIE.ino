/*  NETPIE ESP8266 basic sample                            */
/*  More information visit : https://netpie.io             */

#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_ADS1015.h>
#include "DHT.h"


const char* ssid     = "ampere";
const char* password = "espertap";

//#define APPID   "RoyalProject"
//#define KEY     "2glLm80GxdiQoDt"
//#define SECRET  "5n42hrR3eoynJZdJyv93RH05U"

#define APPID   "rpfew30021001"
#define KEY     "FmB2CDjrdubJAWO"
#define SECRET  "KASG7Ut1TbVXqJLt56nBEzARQ"

#define ALIAS   "rjSensor1"


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
const long intervalSpeed = 5000;
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

  /* Call onMsghandler() when new message arraives */
  microgear.on(MESSAGE, onMsghandler);

  /* Call onFoundgear() when new gear appear */
  microgear.on(PRESENT, onFoundgear);

  /* Call onLostgear() when some gear goes offline */
  microgear.on(ABSENT, onLostgear);

  /* Call onConnected() when NETPIE connection is established */
  microgear.on(CONNECTED, onConnected);

  Serial.begin(115200);
  Serial.println("Starting...");

  /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
  /* You may want to use other method that is more complicated, but provide better user experience */
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  /* Initial with KEY, SECRET and also set the ALIAS here */
  microgear.init(KEY, SECRET, ALIAS);

  /* connect to NETPIE to a specific APPID */
  microgear.connect(APPID);



  dht.begin();
  Wire.begin(D1, D2);
  ads.begin();
  LastValue = 1;

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

  pinMode(RainDropPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(RainDropPin), countRain, FALLING);

}

void loop() {
  /* To check if the microgear is still connected */
  if (microgear.connected()) {
    microgear.loop();
    unsigned long curMillis = millis();
    if (curMillis - preMillis >= intervalSpeed) {
      preMillis = curMillis;

      Rotations = 0;
      sei(); // Enables interrupts
      delay(1000);
      cli(); // Disable interrupts

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


      Serial.print("Rain rate: ");
      Serial.print(rainrate);
      Serial.print(" mm/hr ");

      // clear rain drop
      rainrate = 0;

      float t = dht.readTemperature();
      float h = dht.readHumidity();

      Serial.print("Temp : "); Serial.print(t);
      Serial.print("\tHumid : "); Serial.print(h);

      //    Serial.print(Rotations); Serial.print("\t");
      Serial.print("\tWind Speed : ");
      Serial.print(WindSpeed); Serial.print("\t");
      //    Serial.print(VaneValue); Serial.print("\t");
      //    Serial.print(CalDirection); Serial.print("\t");
      Serial.print("Direction : ");
      getHeading(CalDirection);


      // send to netpie
      if(!isnan(t)) microgear.chat("rjSensor1/temp", String(t));
      if(!isnan(h)) microgear.chat("rjSensor1/humid", String(h));
      microgear.chat("rjSensor1/rain", String(rainrate));
      microgear.chat("rjSensor1/wind", String(WindSpeed));
      microgear.chat("rjSensor1/direc", String(windDirection));

      
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
