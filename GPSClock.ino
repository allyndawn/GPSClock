// GPSClock
// by Allen Snook (allendav) allendav.com
// 19 Jan 2015

// Hardware:
// Arduino UNO
// Adafruit Ultimate GPS Breakout - https://www.adafruit.com/products/746
// Adafruit 0.56" 4-digit 7-segment display with I2C backpack - https://www.adafruit.com/product/881

// Connections:
// UNO -> GPS
// 5V -> VIN
// GND -> GND
// Digital 2 -> RX
// Digital 3 -> TX

// UNO -> LED
// 5V -> VCC+
// GND -> GND
// Analog 5 -> CLK (I2C Clock)
// Analog 4 -> DAT (I2C Data)


// Libraries Required:
// https://github.com/adafruit/Adafruit-GPS-Library
// https://github.com/adafruit/Adafruit-LED-Backpack-Library
// https://github.com/adafruit/Adafruit-GFX-Library

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Wire.h> // Arduino UNO and Mega
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

SoftwareSerial mySerial(3, 2); // RX pin, TX pin
Adafruit_GPS GPS(&mySerial);
Adafruit_7segment matrix = Adafruit_7segment();
unsigned long lastClockUpdateMillis;
bool colonOn;

void setup()  
{    
  Serial.begin(9600);

  lastClockUpdateMillis = 0;
  colonOn = false;

  matrix.begin(0x70);

  GPS.begin(9600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ); // Once every 5 seconds
}

void maybeUpdateClock() {
  // If we've updated the clock within the last second, don't bother doing it now
  unsigned long timeNow = millis();
  if ((timeNow < lastClockUpdateMillis) || (lastClockUpdateMillis + 1000 < timeNow)) {
    lastClockUpdateMillis = timeNow;

    colonOn = ! colonOn;

    int currentHour = GPS.hour;
    int currentMinute = GPS.minute;
    
    int hourTens = currentHour / 10;
    int hourOnes = currentHour % 10;
    int minuteTens = currentMinute / 10;
    int minuteOnes = currentMinute % 10;
    
    matrix.writeDigitNum(0, hourTens);
    matrix.writeDigitNum(1, hourOnes);
    matrix.drawColon(colonOn);
    matrix.writeDigitNum(3, minuteTens);
    matrix.writeDigitNum(4, minuteOnes);
    matrix.writeDisplay();
  }
}

void loop() // run over and over again
{
  GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  maybeUpdateClock();
}
