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


typedef enum class ScannerState
{
  IDLE              = 0,
  ATTRACT           = 1,
  SCAN_DOWN         = 2,
  SCAN_TANSITION    = 3,
  SCAN_UP           = 4,
  SHUFFLE_SCAN      = 5,
  TIKTOK_SCAN       = 6,
  UNLOCKED          = 7,
  SUIT_SHOWCASE     = 8,
  COOLDOWN          = 9

} scanner_state_;

uint16_t stateTime[] = 
{
  0,                // IDLE
  0,                // ATTRACT
  0,                // SCAN_DOWN
  0,                // SCAN_TANSITION
  0,                // SCAN_UP
  0,                // SHUFFLE_SCAN
  0,                // TIKTOK_SCAN
  0,                // UNLOCKED
  0,                // SUIT_SHOWCASE
  0                 // COOLDOWN
};


ScannerState myState = ScannerState::IDLE;

const uint8_t historyDepth = 10;
ScannerState stateHistory[historyDepth];


// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lastTouched = 0;
uint16_t currTouched = 0;



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


void loop() {

  switch (myState) {
    case ScannerState::IDLE:
      break;
    case ScannerState::ATTRACT:
      break;
    case ScannerState::SCAN_DOWN:
      break;
    case ScannerState::SCAN_TANSITION:
      break;
    case ScannerState::SCAN_UP:
      break;
    case ScannerState::SHUFFLE_SCAN:
      break;
    case ScannerState::TIKTOK_SCAN:
      break;
    case ScannerState::UNLOCKED:
      break;
    case ScannerState::SUIT_SHOWCASE:
      break;
    case ScannerState::COOLDOWN:
      break;
    default:
      break;
  }

  // Get the currently touched pads
  currTouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currTouched & _BV(i)) && !(lastTouched & _BV(i)) ) {
      DEBUG_PRINT(i); DEBUG_PRINTLN(" touched");
      startHandScan();
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currTouched & _BV(i)) && (lastTouched & _BV(i)) ) {
      DEBUG_PRINT(i); DEBUG_PRINTLN(" released");
      stopHandScan();
    }
  }

  // reset our state
  lastTouched = currTouched;

  scannerLoop();
  
  delay(10);

}

void setState(ScannerState newState) {

  myState = newState;

  String stateString;
  switch (myState) {
    case ScannerState::IDLE:
      startIdle();
      stateString = "IDLE";
      break;
    case ScannerState::ATTRACT:
      startAttract();
      stateString = "ATTRACT";
      break;
    case ScannerState::SCAN_DOWN:
      startScanDown();
      stateString = "SCAN_DOWN";
      break;
    case ScannerState::SCAN_TANSITION:
      startScanTransition();
      stateString = "SCAN_TANSITION";
      break;
    case ScannerState::SCAN_UP:
      startScanUp();
      stateString = "SCAN_UP";
      break;
    case ScannerState::SHUFFLE_SCAN:
      startShuffleScan();
      stateString = "SHUFFLE_SCAN";
      break;
    case ScannerState::TIKTOK_SCAN:
      startTikTokScan();
      stateString = "TIKTOK_SCAN";
      break;
    case ScannerState::UNLOCKED:
      startUnlocked();
      stateString = "UNLOCKED";
      break;
    case ScannerState::SUIT_SHOWCASE:
      startSuitShowcase();
      stateString = "SUIT_SHOWCASE";
      break;
    case ScannerState::COOLDOWN:
      startCooldown();
      stateString = "COOLDOWN";
      break;
    default:
      stateString = "404 - File Not Found";
      break;
  }
  
  DEBUG_PRINT("Scanner State = ");
  DEBUG_PRINTLN(stateString);

  updateStateHistory();
  // if (checkForGordonCodeMatch()) {
  //   setState(GORDON);
  // }
  // else if (checkForRebootCodeMatch()) {
  //   doReboot();
  // }
}


void startIdle() {

}


void startAttract() {

}


void startScanDown() {

}


void startScanTransition() {

}


void startScanUp() {

}


void startShuffleScan() {

}


void startTikTokScan() {

}


void startUnlocked() {

}


void startSuitShowcase() {

}


void startCooldown() {

}


void updateStateHistory() {

  for (int i = historyDepth-1; i >= 1; --i) {
    stateHistory[i] = stateHistory[i-1];
  }

  stateHistory[0] = myState;

  #ifdef DEBUG
  DEBUG_PRINT("StateHistory: ");

  for (int i = 0; i < historyDepth; ++i) {
    DEBUG_PRINT((int)stateHistory[i]);

    if (i < 9) {
      DEBUG_PRINT(", ");
    }
  }

  DEBUG_PRINTLN("");
  #endif
}


void doReboot() {
  SCB_AIRCR = 0x05FA0004;
}

