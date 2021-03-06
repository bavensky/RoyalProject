#include <Wire.h>
#include <math.h>
#include <Adafruit_ADS1015.h>
#include "DHT.h"



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
const long intervalSpeed = 1000;



// init rain gauge
const byte RainDropPin = 9;
const int intervalRain = 500;
volatile unsigned long tiptime = millis();
unsigned long tipcount;
double rainrate;



// wind speed count
void isr_rotation () {
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}


// rain drop count
void countRain() {
  unsigned long curtime = millis();

  if ((curtime - tiptime) < intervalRain) {
    return;
  }

  tipcount = curtime - tiptime;
  tiptime = curtime;

  rainrate = 914400.0 / tipcount;
}


// Converts compass direction to heading
void getHeading(int direction) {
  if (direction < 22)
    Serial.println("N");
  else if (direction < 67)
    Serial.println("NE");
  else if (direction < 112)
    Serial.println("E");
  else if (direction < 157)
    Serial.println("SE");
  else if (direction < 212)
    Serial.println("S");
  else if (direction < 247)
    Serial.println("SW");
  else if (direction < 292)
    Serial.println("W");
  else if (direction < 337)
    Serial.println("NW");
  else
    Serial.println("N");
}




void setup() {
  Serial.begin(9600);
  delay(2000);
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
  }



}
