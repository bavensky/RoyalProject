#define RainDropPin    10 // GPIO10 | SD3

// init rain gauge variable
const int intervalRain = 500;
volatile unsigned long tiptime = millis();
unsigned long tipcount;
double rainrate;

//// rain drop count
void countRain() {
  unsigned long curtime = millis();

  if ((curtime - tiptime) < intervalRain) {
    return;
  }

  tipcount = curtime - tiptime;
  tiptime = curtime;

  rainrate = 914400.0 / tipcount;
  Serial.print("rainrate = ");
  Serial.println(rainrate);
}

void setup() {
  Serial.begin(115200);
  Serial.print("Serial begin");
  delay(2000);
  
  
  pinMode(RainDropPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(RainDropPin), countRain, FALLING);
}

void loop() {
  // put your main code here, to run repeatedly:

}
