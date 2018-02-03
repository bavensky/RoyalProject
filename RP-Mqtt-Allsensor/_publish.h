#include <Arduino.h>
#include <MqttConnector.h>
#include <DHT.h>

extern int relayPinState;
extern MqttConnector* mqtt;
extern int relayPin;
extern char myName[];
extern DHT dht;

//extern const byte WindSensorPin  = D2;
//extern const byte RainDropPin    = D3;

static void readSensor();
static void isr_rotation();
static void countRain();
static void getHeading();

// sensor
float temperature = 0;
float humidity = 0;

// init wind speed variable
volatile unsigned long Rotations;
volatile unsigned long ContactBounceTime;
const long intervalSpeed = 1000;
unsigned long preMillis = 0;
float WindSpeed;


// init direction variable
int VaneValue;// raw analog value from wind vane
int Direction;// translated 0 - 360 direction
int CalDirection;// converted value with offset applied
int LastValue;
String WindDirection = "";



// init rain gauge variable
const int intervalRain = 500;
volatile unsigned long tiptime = millis();
unsigned long tipcount;
double rainrate;


//####################SUB Void##############################
// wind speed count
void isr_rotation () {
  if ((millis() - ContactBounceTime) > 15 ) { // debounce the switch contact.
    Rotations++;
    ContactBounceTime = millis();
  }
}


//// rain drop count
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
  if (direction < 22) {
    //    Serial.println("N");
    WindDirection = "N";
  } else if (direction < 67) {
    //    Serial.println("NE");
    WindDirection = "NE";
  } else if (direction < 112) {
    //    Serial.println("E");
    WindDirection = "E";
  } else if (direction < 157) {
    //    Serial.println("SE");
    WindDirection = "SE";
  } else if (direction < 212) {
    //    Serial.println("S");
    WindDirection = "S";
  } else if (direction < 247) {
    //    Serial.println("SW");
    WindDirection = "SW";
  } else if (direction < 292) {
    //    Serial.println("W");
    WindDirection = "W";
  } else if (direction < 337) {
    //    Serial.println("NW");
    WindDirection = "NW";
  } else {
    //    Serial.println("N");
    WindDirection = "N";
  }
}



extern String DEVICE_NAME;
extern int PUBLISH_EVERY;

void register_publish_hooks() {
  strcpy(myName, DEVICE_NAME.c_str());
  mqtt->on_prepare_data_once([&](void) {
    Serial.println("initializing sensor...");
  });

  mqtt->on_before_prepare_data([&](void) {
    readSensor();


    // wind speed
    Rotations = 0;
    sei(); // Enables interrupts
    delay(1000);
    cli(); // Disable interrupts
    WindSpeed = Rotations * 0.75;

    // direction
    VaneValue = map(ads.readADC_SingleEnded(3), 0, 17550, 0, 1023);
    Direction = map(VaneValue, 0, 1023, 0, 360);
    CalDirection = Direction + Offset;
    if (CalDirection > 360) {
      CalDirection = CalDirection - 360;
    } else if (CalDirection < 0) {
      CalDirection = CalDirection + 360;
    }
    getHeading(CalDirection);



    //      Serial.print("Rain rate: ");
    //      Serial.print(rainrate);
    //      Serial.print(" mm/hr");
    //      rainrate = 0; // clear rainrate
    //      Serial.print("\tTemp : ");          Serial.print(temperature);
    //      Serial.print("\tHumid : ");       Serial.print(humidity);
    //      Serial.print("\tWind Speed : ");  Serial.print(WindSpeed);
    //      Serial.print("\tDirection : ");   getHeading(CalDirection);
    //    }

  });

  mqtt->on_prepare_data([&](JsonObject * root) {
    JsonObject& data = (*root)["d"];
    JsonObject& info = (*root)["info"];
    data["myName"] = myName;
    data["millis"] = millis();
    data["temperature"] = temperature;
    data["humidity"] = humidity;
    data["rainrate"] = rainrate;
    data["windspeed"] = WindSpeed;
    data["direction"] = WindDirection;

    rainrate = 0; // clear rain drop
    
    //    data["relayState"] = relayPinState;
    //    data["updateInterval"] = PUBLISH_EVERY;
    //    data["A0"] = analogRead(A0);
  }, PUBLISH_EVERY);

  mqtt->on_after_prepare_data([&](JsonObject * root) {
    /**************
      JsonObject& data = (*root)["d"];
      data.remove("version");
      data.remove("subscription");
    **************/
  });
}

static void readSensor() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    temperature = t;
    humidity = h;
  }

}
