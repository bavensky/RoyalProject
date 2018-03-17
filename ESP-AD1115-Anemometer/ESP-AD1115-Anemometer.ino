#include <Wire.h>
#include <math.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1115 ads(0x48);

#define Offset 0
#define WindSensorPin D3


int VaneValue;// raw analog value from wind vane
int Direction;// translated 0 - 360 direction
int CalDirection;// converted value with offset applied
int LastValue;


volatile unsigned long Rotations;
volatile unsigned long ContactBounceTime;
float WindSpeed;


void isr_rotation () {
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
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

  Wire.begin(D1, D2);
  ads.begin();
  LastValue = 1;

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

}


void loop() {
  VaneValue = map(ads.readADC_SingleEnded(3), 0, 17550, 0, 1023);
  Direction = map(VaneValue, 0, 1023, 0, 360);
  CalDirection = Direction + Offset;

  if (CalDirection > 360) {
    CalDirection = CalDirection - 360;
  } else if (CalDirection < 0) {
    CalDirection = CalDirection + 360;
  }


  // Only update the display if change greater than 2 degrees.
  //  if (abs(CalDirection - LastValue) > 5)
  //  {

  Rotations = 0;
  sei();
  //  delay (1000);
  //  cli();
  // convert to mp/h using the formula V=P(2.25/T)
  // V = P(2.25/3) = P * 0.75

  WindSpeed = Rotations * 0.75;

  Serial.print(Rotations); Serial.print("\t\t");
  Serial.print(WindSpeed); Serial.print("\t\t");
  Serial.print(VaneValue); Serial.print("\t\t");
  Serial.print(CalDirection); Serial.print("\t\t");
  getHeading(CalDirection);

  //  LastValue = CalDirection;
  //  }
  //  delay(500);
}

