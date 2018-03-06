/*
 

 
 ** SDA   -   pin D1
 ** SCL   -   pin D2


 ** CLK   -   pin D5
 ** MISO  -   pin D6
 ** MOSI  -   pin D7
 ** CS    -   pin D8



*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <BH1750.h>
#include "RTClib.h"

BH1750 lightMeter;
RTC_DS1307 rtc;

File myFile;


char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};



void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Wire.begin(D1, D2);
  lightMeter.begin();

  rtc.begin();
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


  if (!SD.begin(D8)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");


  myFile = SD.open("test.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("Date, Time, Light");
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening test.txt");
  }
  delay(2000);
}

void loop()
{
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();


  uint16_t lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");


  myFile = SD.open("test.csv", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.print(now.day()); myFile.print("/");
    myFile.print(now.month()); myFile.print("/");
    myFile.print(now.year()); myFile.print(",");
    myFile.print(now.hour()); myFile.print(":");
    myFile.print(now.minute()); myFile.print(":");
    myFile.print(now.second()); myFile.print(",");
    myFile.print(lux);
    myFile.println("");
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening test.txt");
  }

  delay(5000);

}


