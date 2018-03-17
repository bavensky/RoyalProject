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
  if (direction < 22) {
    Serial.println("N");
    windDirection = "N";
  }
  else if (direction < 67) {
    Serial.println("NE");
    windDirection = "NE";
  }
  else if (direction < 112) {
    Serial.println("E");
    windDirection = "N";
  }
  else if (direction < 157) {
    Serial.println("SE");
    windDirection = "SE";
  }
  else if (direction < 212) {
    Serial.println("S");
    windDirection = "S";
  }
  else if (direction < 247) {
    Serial.println("SW");
    windDirection = "SW";
  }
  else if (direction < 292) {
    Serial.println("W");
    windDirection = "W";
  }
  else if (direction < 337) {
    Serial.println("NW");
    windDirection = "NW";
  }
  else {
    Serial.println("N");
    windDirection = "N";
  }
}
