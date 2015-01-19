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

//
//
// Includes for GPS Library
//
//

#include <Adafruit_GPS.h>
#if ARDUINO >= 100
 // http://arduino.cc/en/Reference/softwareSerial
 #include <SoftwareSerial.h>
#else
 // Older Arduino IDE requires NewSoftSerial, download from:
 // http://arduiniana.org/libraries/newsoftserial/
 // #include <NewSoftSerial.h>
 // DO NOT install NewSoftSerial if using Arduino 1.0 or later!
#endif

//
//
// Includes for LED Libraries
//
//

#include <Wire.h> // Arduino UNO and Mega
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

//
//
// Global constants
//
//

// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

//
//
// Create GPS singleton
//
//

#if ARDUINO >= 100
  SoftwareSerial mySerial(3, 2); // RX pin, TX pin
#else
  NewSoftSerial mySerial(3, 2);
#endif
Adafruit_GPS GPS(&mySerial);

//
//
// Create LED singleton
//
//

Adafruit_7segment matrix = Adafruit_7segment();

//
//
// Global variables
//
//

int currentHour;
int currentMinute;
int currentSeconds;

//
//
// Func prototype keeps Arduino 0023 happy
//
//

boolean usingInterrupt = false;
void useInterrupt(boolean); 

void setup()  
{    
  Serial.begin(115200);
  Serial.println("GPSClock");
  
  currentHour = 0;
  currentMinute = 0;
  currentSeconds = 0;
  
  matrix.begin(0x70);
  delay(500);

  GPS.begin(9600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
  GPS.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
  
  useInterrupt(true);
  
  delay(1000);
}

//
//
// Interrupt is called once a millisecond, looks for any new GPS data, and stores it
// writing direct to UDR0 is much much faster than Serial.print 
// but only one character can be written at a time. 
SIGNAL(TIMER0_COMPA_vect) {
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO) {
    if (c) {
      UDR0 = c;
    }
  }
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}


void loop() // run over and over again
{
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }
  
  if (GPS.seconds != currentSeconds) {
    currentHour = GPS.hour;
    currentMinute = GPS.minute;
    currentSeconds = GPS.seconds;
    
    int hourTens = currentHour / 10;
    int hourOnes = currentHour % 10;
    int minuteTens = currentMinute / 10;
    int minuteOnes = currentMinute % 10;
    
    matrix.writeDigitNum(0, hourTens);
    matrix.writeDigitNum(1, hourOnes);
    matrix.drawColon(0 == currentSeconds % 2);
    matrix.writeDigitNum(3, minuteTens);
    matrix.writeDigitNum(4, minuteOnes);
    matrix.writeDisplay();
  }
}

