#define DEBUG 1

#include <Easing.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_AW9523.h>
#include "Adafruit_MPR121.h"


#ifdef DEBUG
# define DEBUG_PRINTLN(x) Serial.println(x);
# define DEBUG_PRINT(x) Serial.print(x);
#else
# define DEBUG_PRINTLN(x)
# define DEBUG_PRINT(x)
#endif

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();
Adafruit_MPR121 cap = Adafruit_MPR121();


typedef enum ScannerState
{
  IDLE,
  ATTRACT,
  SCAN_DOWN,
  SCAN_UP,
  SHUFFLE_SCAN,
  TIKTOK_SCAN,
  UNLOCKED,
  SUIT_SHOWCASE,
  COOLDOWN

} scanner_state_;


// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;



void setup() {
  Serial.begin(9600);
  //while (!Serial) delay(1);  // wait for serial port to open
  
  Serial.println("Hand Scanner Initializing");

  scannerSetup();


  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1); delay(10);  // halt forever
  }
  Serial.println("MPR121 found!");
  cap.setThreshholds(3, 3);


  Serial.println("Hand Scanner Initialized");
}


int animationToRun = 100;

bool direction = true;

void loop() {


   if (Serial.available() > 0) {
    if (Serial.available() > 0) {
      Serial.read();
      Serial.println("Still Avail");
    }
    Serial.flush();
    //Serial.println(freq);
    animationToRun = Serial.parseInt();
  }


  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" touched");
      direction = true;
      startHandScan();
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.println(" released");
      direction = false;
      stopHandScan();
    }
  }

  // reset our state
  lasttouched = currtouched;


  scannerLoop();
  



  delay(10);

}