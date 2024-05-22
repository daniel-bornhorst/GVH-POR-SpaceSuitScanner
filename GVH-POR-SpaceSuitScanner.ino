#define DEBUG 1

#include <Easing.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_AW9523.h>
#include "Adafruit_MPR121.h"
#include "autonet.h"

#include <SPI.h>


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


// MY network settings
// ALWAYS EDIT
IPAddress my_ip(10,32,16,33);                                 // Arduino/Teensy IP. Must be unique
byte my_mac[] = {0x56, 0x5A, 0x69, 0x66, 0x66, 0x61};         // Arduino/Teebsy. Must be uniqe. https://miniwebtool.com/mac-address-generator/
char my_name[50] = "2c0f57d0-ee22-4aa0-8184-d64c4f9ec0a1";    // Unique ID for each device. generate here https://www.uuidgenerator.net/version4
const unsigned int  my_localPort = 6667;                      // Can be set in Jefe Device Qsys plugin. Default is 6667


// Network settings for Jefe Show controller and redudant core (QSys Core 101 and 104)
// SOMETIMES EDIT
IPAddress jefe_ip(10, 32, 16, 10);          // Primary Core
// IPAddress jefe_ip2(10, 32, 16, 13);      // Redundant Core
IPAddress jefe_ip2(10, 32, 16, 14);         // Test Core
const unsigned int jefe_port = 6666;        // Jefe Port 


// Stuff we need if we're not on 10.32.16.*
byte ddns[] =     {10, 32, 16, 1};
byte *gateway =   ddns;
byte subnet[] =   {255, 255, 240, 0};


// Network settings for OSC messages. Typically Mac Mini Media Server (or OSC destination)
// ALWAYS EDIT
IPAddress remote_ip(10, 32, 70, 26);    // Audio recieving IP
const unsigned int remote_port = 9000;   // receiving port 


// Create your Autonet object, pass in the network settings from
Autonet autonet(my_ip, my_mac, my_name, ddns, subnet, jefe_ip, jefe_port, jefe_ip2, jefe_port, 1);
EthernetUDP Udp;
int is_connected = 1;               // 0 = broken connection, 1 = connected. Not required, but can be handy



typedef enum class ScannerState
{
  IDLE              = 0,
  ATTRACT           = 1,
  SCAN_DOWN         = 2,
  SCAN_UPDOWNTRANS  = 3,
  SCAN_UP           = 4,
  SCAN_UPSHUFFTRANS = 5,
  SHUFFLE_SCAN      = 6,
  SCAN_SHUFFTODEEP  = 7,
  DEEP_SCAN         = 8,
  SCAN_DEEPTOTIKTOK = 9,
  TIKTOK_SCAN       = 10,
  UNLOCKED          = 11,
  SUIT_SHOWCASE     = 12,
  COOLDOWN          = 13

} scanner_state_;

uint16_t stateTime[] = 
{
  0,                // IDLE
  0,                // ATTRACT
  500,              // SCAN_DOWN
  10,               // SCAN_UPDOWNTRANS
  500,              // SCAN_UP
  500,              // SCAN_UPSHUFFTRANS
  10,               // SHUFFLE_SCAN
  500,              // SCAN_SHUFFTODEEP
  250,              // DEEP_SCAN
  500,              // SCAN_DEEPTOTIKTOK
  400,              // TIKTOK_SCAN
  10000,            // UNLOCKED
  0,                // SUIT_SHOWCASE
  0                 // COOLDOWN
};


ScannerState myState = ScannerState::IDLE;

const uint8_t historyDepth = 10;
ScannerState stateHistory[historyDepth];

const uint8_t upDownCount = 1;
uint8_t upDownCounter = 0;

const uint8_t shuffleCount = 75;
uint8_t shuffleCounter = 0;

const uint8_t deepCount = 9;
uint8_t deepCounter = 0;

const uint8_t tikTokCount = 5;
uint8_t tikTokCounter = 0;


// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lastTouched = 0;
uint16_t currTouched = 0;


elapsedMillis stateTimer;

elapsedMillis capTouchPollTimer;
long unsigned int capTouchPollRate = 100;


long unsigned int matrixRefreshRate = 30;
int matrixMode = 3;
int maxtrixRandomizer = 7;


void setup() {
  Serial.begin(9600);
  //while (!Serial) delay(1);  // wait for serial port to open
  
  Serial.println("Hand Scanner Initializing");

  scannerSetup();

  matrixSetup();


  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1); delay(10);  // halt forever
  }
  Serial.println("MPR121 found!");
  cap.setThreshholds(3, 3);
  capTouchPollTimer = 0;

  Ethernet.begin(my_mac, my_ip, ddns, gateway, subnet);
  Udp.begin(my_localPort);
  autonet.setup(Udp);


  Serial.println("Hand Scanner Initialized");
}


void loop() {

  autonet.loop();

  matrixLoop();

  switch (myState) {
    case ScannerState::IDLE:
      break;
    case ScannerState::ATTRACT:
      break;
    case ScannerState::SCAN_DOWN:
      scanDownLoop();
      break;
    case ScannerState::SCAN_UPDOWNTRANS:
      if (stateTimer > stateTime[(int)ScannerState::SCAN_UPDOWNTRANS]) {
        setState(ScannerState::SCAN_UP);
      }
      break;
    case ScannerState::SCAN_UP:
      scanUpLoop();
      break;
    case ScannerState::SCAN_UPSHUFFTRANS:
      if (stateTimer > stateTime[(int)ScannerState::SCAN_UPSHUFFTRANS]) {
        setState(ScannerState::SHUFFLE_SCAN);
      }
      break;
    case ScannerState::SHUFFLE_SCAN:
      if (stateTimer > stateTime[(int)ScannerState::SHUFFLE_SCAN] && shuffleCounter < shuffleCount) {
        shuffleScanLoop();
        shuffleCounter++;
        stateTimer = 0;
      }
      else if (shuffleCounter >= shuffleCount) { setState(ScannerState::SCAN_SHUFFTODEEP); }
      break;
    case ScannerState::SCAN_SHUFFTODEEP:
      if (stateTimer > stateTime[(int)ScannerState::SCAN_SHUFFTODEEP]) {
        setState(ScannerState::DEEP_SCAN);
      }
      break;
    case ScannerState::DEEP_SCAN:
      if (stateTimer > stateTime[(int)ScannerState::DEEP_SCAN] && deepCounter < deepCount) {
        deepScanLoop();
        deepCounter++;
        stateTimer = 0;
      }
      else if (deepCounter >= deepCount) {
        setState(ScannerState::UNLOCKED);
      }
      break;
    case ScannerState::SCAN_DEEPTOTIKTOK:
      if (stateTimer > stateTime[(int)ScannerState::SCAN_DEEPTOTIKTOK]) {
        setState(ScannerState::TIKTOK_SCAN);
      }
      break;
    case ScannerState::TIKTOK_SCAN:
      if (stateTimer > stateTime[(int)ScannerState::TIKTOK_SCAN] && tikTokCounter < tikTokCount) {
        tikTokScanLoop();
        tikTokCounter++;
        stateTimer = 0;
      }
      else if (tikTokCounter >= tikTokCount) { setState(ScannerState::UNLOCKED); }
      break;
    case ScannerState::UNLOCKED:
      if (stateTimer > stateTime[(int)ScannerState::UNLOCKED]) {
        setState(ScannerState::IDLE);
      }
      break;
    case ScannerState::SUIT_SHOWCASE:
      break;
    case ScannerState::COOLDOWN:
      break;
    default:
      break;
  }
  
  if (capTouchPollTimer > capTouchPollRate) {

    // Get the currently touched pads
    currTouched = cap.touched();

    // // Scan all Cap Touch Inputs for Changed state
    // for (uint8_t i=0; i<12; i++) {
    //   // it if *is* touched and *wasnt* touched before, alert!
    //   if ((currTouched & _BV(i)) && !(lastTouched & _BV(i)) ) {
    //     DEBUG_PRINT(i); DEBUG_PRINTLN(" touched");
    //     setState(ScannerState::SCAN_DOWN);
    //   }
    //   // if it *was* touched and now *isnt*, alert!
    //   if (!(currTouched & _BV(i)) && (lastTouched & _BV(i)) ) {
    //     DEBUG_PRINT(i); DEBUG_PRINTLN(" released");
    //     setState(ScannerState::IDLE);
    //     stopHandScan();
    //   }
    // }

    for (uint8_t i=0; i<2; i++) {
      // it if *is* touched and *wasnt* touched before, alert!
      if ((currTouched & _BV(i)) && !(lastTouched & _BV(i)) ) {
        DEBUG_PRINT(i); DEBUG_PRINTLN(" touched");
        setState(ScannerState::SCAN_DOWN);
      }
      // if it *was* touched and now *isnt*, alert!
      if (!(currTouched & _BV(i)) && (lastTouched & _BV(i)) ) {
        DEBUG_PRINT(i); DEBUG_PRINTLN(" released");
        if (myState != ScannerState::UNLOCKED) {
          setState(ScannerState::IDLE);
          stopHandScan();
        }
      }
    }

    capTouchPollTimer = 0;
  }
  

  // reset our state
  lastTouched = currTouched;

  
  
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
    case ScannerState::SCAN_UPDOWNTRANS:
      startScanUpDownTransition();
      stateString = "SCAN_UPDOWNTRANS";
      break;
    case ScannerState::SCAN_UP:
      startScanUp();
      stateString = "SCAN_UP";
      break;
    case ScannerState::SCAN_UPSHUFFTRANS:
      startScanUpToShuffleTransition();
      stateString = "SCAN_UPSHUFFTRANS";
      break;
    case ScannerState::SHUFFLE_SCAN:
      startShuffleScan();
      stateString = "SHUFFLE_SCAN";
      break;
    case ScannerState::SCAN_SHUFFTODEEP:
      startSuffleToDeepTransition();
      stateString = "SHUFFLE_TO_DEEP";
      break;
    case ScannerState::DEEP_SCAN:
      stateString = "DEEP_SCAN";
      startDeepScan();
      break;
    case ScannerState::SCAN_DEEPTOTIKTOK:
      startDeepToTikTokeTransition();
      stateString = "SCAN_DEEP_TO_TIK_TOK";
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


void sendOSCMessage(){
  OSCMessage outMsg("/fp/spacesuittest");
  //outMsg.add(doorbell);
  is_connected = autonet.sendOSC(&outMsg, remote_ip, remote_port);
}


void startIdle() {
  matrixRefreshRate = 30;
  matrixMode = 3;
  maxtrixRandomizer = 7;
}


void startAttract() {

}


void startScanDown() {
  //upDownCounter = 0;
  matrixMode = 1;
  matrixRefreshRate = 70;
  startHandScanDown();
}


void startScanUpDownTransition() {
  stateTimer = 0;
}


void startScanUp() {
  matrixMode = 1;
  matrixRefreshRate = 30;
  startHandScanUp();
}


void startScanUpToShuffleTransition() {
  matrixMode = 0;
  stateTimer = 0;
}


void startShuffleScan() {
  matrixMode = 1;
  matrixRefreshRate = 120;

  shuffleCounter = 0;
  startShuffleHandScan();
  stateTimer = 0;
}


void startSuffleToDeepTransition() {
  clearAllOutputs();
  matrixMode = 0;
  stateTimer = 0;
}


void startDeepScan() {
  matrixMode = 2;
  matrixRefreshRate = 5;
  startDeepHandScan();
  deepCounter = 0;
}


void startDeepToTikTokeTransition() {
  //clearAllOutputs();
  stateTimer = 0;
}


void startTikTokScan() {
  tikTokCounter = 0;
  startTikTokHandScan();
  stateTimer = 0;
}


void startUnlocked() {
  matrixMode = 3;
  matrixRefreshRate = 30;
  maxtrixRandomizer = 1;

  clearAllOutputs();
  stateTimer = 0;
  
  sendOSCMessage();
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

