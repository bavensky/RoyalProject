void firstSave() {
  myFile = SD.open("DATALOGER.csv", FILE_WRITE);
  if (myFile) {
    myFile.println("Date, Time, Temperature, Humidity, Raindrop, WindSpeed, WindDirection, LightLevel, Voltage");
    myFile.close();
    Serial.println("Write done.");
  } else {
    Serial.println("Error opening sd card");
  }
}

void dataLog(String _temp, String _humid, String _rain, String _wind, String _direc, String _lux, String _volt ) {
  myFile = SD.open("DATALOGER.csv", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to sd card...");
    DateTime now = rtc.now();
    myFile.print(now.day()); myFile.print("/");
    myFile.print(now.month()); myFile.print("/");
    myFile.print(now.year()); myFile.print(",");
    myFile.print(now.hour()); myFile.print(":");
    myFile.print(now.minute()); myFile.print(":");
    myFile.print(now.second()); myFile.print(",");
    myFile.print(_temp); myFile.print(",");
    myFile.print(_humid); myFile.print(",");
    myFile.print(_wind); myFile.print(",");
    myFile.print(_direc); myFile.print(",");
    myFile.print(_lux); myFile.print(",");
    myFile.print(_volt);
    myFile.println("");
    myFile.close();
    Serial.println("done.");
  } else {
    Serial.println("Error opening sd card");
  }
}

