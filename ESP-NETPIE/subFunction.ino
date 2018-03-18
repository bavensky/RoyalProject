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
    windDirection = "N";
  }
  else if (direction < 67) {
    windDirection = "NE";
  }
  else if (direction < 112) {
    windDirection = "N";
  }
  else if (direction < 157) {
    windDirection = "SE";
  }
  else if (direction < 212) {
    windDirection = "S";
  }
  else if (direction < 247) {
    windDirection = "SW";
  }
  else if (direction < 292) {
    windDirection = "W";
  }
  else if (direction < 337) {
    windDirection = "NW";
  }
  else {
    windDirection = "N";
  }
}
