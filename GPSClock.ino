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

// 13 -> Piezo +
// GND -> Piezo GND

// Libraries Required:
// https://github.com/adafruit/Adafruit-GPS-Library
// https://github.com/adafruit/Adafruit-LED-Backpack-Library
// https://github.com/adafruit/Adafruit-GFX-Library

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Wire.h> // Arduino UNO and Mega
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#include "ChimeArray.h"

SoftwareSerial mySerial(3, 2); // RX pin, TX pin
Adafruit_GPS GPS(&mySerial);
Adafruit_7segment matrix = Adafruit_7segment();

ChimeArray *chimeArray;

bool bTimeAvailable;
unsigned long lastClockUpdateMillis;
unsigned long nextClockChimeMillis;
bool colonOn;
short lastChimePlayed;

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

void enqueueChime(unsigned short pitch, unsigned short duration = CHIMEARRAY_DURATION_QUARTER) {
  if (nextClockChimeMillis == 0) {
    nextClockChimeMillis = millis() + 500;
  }

  chime_struct new_chime;
  new_chime.pitch = pitch;
  new_chime.duration = duration;
  chimeArray->push(&new_chime);
}

void enqueuePhraseA() {
  enqueueChime(CHIMEARRAY_PITCH_GSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_FSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_E4);
  enqueueChime(CHIMEARRAY_PITCH_B3, 2 * CHIMEARRAY_DURATION_QUARTER);
}

void enqueuePhraseB() {
  enqueueChime(CHIMEARRAY_PITCH_E4);
  enqueueChime(CHIMEARRAY_PITCH_GSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_FSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_B3, 2 * CHIMEARRAY_DURATION_QUARTER);
}

void enqueuePhraseC() {
  enqueueChime(CHIMEARRAY_PITCH_E4);
  enqueueChime(CHIMEARRAY_PITCH_FSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_GSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_E4, 2 * CHIMEARRAY_DURATION_QUARTER);
}

void enqueuePhraseD() {
  enqueueChime(CHIMEARRAY_PITCH_GSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_E4);
  enqueueChime(CHIMEARRAY_PITCH_FSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_B3, 2 * CHIMEARRAY_DURATION_QUARTER);
}

void enqueuePhraseE() {
  enqueueChime(CHIMEARRAY_PITCH_B3);
  enqueueChime(CHIMEARRAY_PITCH_FSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_GSHARP4);
  enqueueChime(CHIMEARRAY_PITCH_E4, 2 * CHIMEARRAY_DURATION_QUARTER);
}

void maybeEnqueueQuarterChime() {
  if (!chimeArray->isEmpty()) {
    return;
  }

  // 1st quarter
  if (15 == GPS.minute) {
    if ( 15 == lastChimePlayed ) {
      return; // Don't play it again
    }
    lastChimePlayed = 15;
    enqueuePhraseA();
    return;
  }

  // 2nd quarter
  if (30 == GPS.minute) {
    if ( 30 == lastChimePlayed ) {
      return; // Don't play it again
    }
    lastChimePlayed = 30;
    enqueuePhraseB();
    enqueuePhraseC();
    return;
  }

  // 3rd quarter
  if (45 == GPS.minute) {
    if ( 45 == lastChimePlayed ) {
      return; // Don't play it again
    }
    lastChimePlayed = 45;
    enqueuePhraseD();
    enqueuePhraseE();
    enqueuePhraseA();
    return;
  }

  // 4th quarter
  if (0 == GPS.minute) {
     if ( 0 == lastChimePlayed ) {
      return; // Don't play it again
    }
    lastChimePlayed = 0;
    enqueuePhraseB();
    enqueuePhraseC();
    enqueuePhraseD();
    enqueuePhraseE();
    // enqueueHourStrikes(GPS.hour); TODO - local timezone, cap at 12
  }
}

void enqueueHourStrikes(unsigned short count) {
  for (unsigned short i=0; i < count; i++) {
    enqueueChime(CHIMEARRAY_PITCH_E3, 6 * CHIMEARRAY_DURATION_QUARTER);
  }
}

void silenceGPS(bool silent) {
  if (silent) {
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);
  } else {
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_ALLDATA);
  }
}

void maybePlayChimes() {
  if (nextClockChimeMillis == 0) {
    return;
  }

  unsigned long timeNow = millis();
  if (nextClockChimeMillis > timeNow) {
    return;
  }

  chime_struct nextChime;
  if (chimeArray->shift(&nextChime)) {
    silenceGPS(true);
    delay(50); // To avoid the GPS silencing command from disturbing the onset of the tone
    tone(13, nextChime.pitch, nextChime.duration);
    nextClockChimeMillis += nextChime.duration + 100;
  } else {
    nextClockChimeMillis = 0;
    silenceGPS(false);
  }
}

void loop() // run over and over again
{
  char c = GPS.read();
  if (c) {
    Serial.print(c);
  }
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      return;
    }
    if (!bTimeAvailable) {
      if ((GPS.hour != 0) && (GPS.minute != 0) && (GPS.seconds != 0)) {
        bTimeAvailable = true;
      }
    }
  }

  if (!bTimeAvailable) {
    return;
  }

  maybeUpdateClock();

  maybeEnqueueQuarterChime();

  maybePlayChimes();
}

void setup()
{
  Serial.begin(9600);

  bTimeAvailable = false;
  lastClockUpdateMillis = 0;
  nextClockChimeMillis = 0;
  colonOn = false;
  lastChimePlayed = -1;

  chimeArray = new ChimeArray();
  chimeArray->setDebugStream(&Serial);

  matrix.begin(0x70);
  matrix.writeDigitRaw(0, 0x40); // "-"
  matrix.writeDigitRaw(1, 0x40);
  matrix.writeDigitRaw(3, 0x40);
  matrix.writeDigitRaw(4, 0x40);
  matrix.writeDisplay();

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ);
  silenceGPS(false);
}
