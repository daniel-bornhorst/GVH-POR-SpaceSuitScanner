#define NUM_NOODS 9
int halfNoods = NUM_NOODS/2;

Adafruit_AW9523 ledDriver;

// long unsigned timingVector[] = {210, 48, 25, 9, 16, 17, 17, 48, 210};  // In Milliseconds

long unsigned timingVector[] = {
  30, 
  30, 
  30, 
  30, 
  30, 
  30, 
  30, 
  30, 
  30};  // In Milliseconds

long unsigned timeBetweenUpAndDownScan = 300;
int scanIndex = 0;

// LED Outs
int indexToOuputMap[] = {1, 2, 3, 4, 5, 6, 7, 14, 15};

elapsedMillis scannerAnimationTimer;

long unsigned int nextFrameTime = 0;

bool tikTokToggle = false;

void scannerSetup() {

  if (! ledDriver.begin(0x58)) {
    Serial.println("LED Driver not found? Check wiring!");
    while (1) delay(10);  // halt forever
  }
  Serial.println("LED Driver found!");


  // Do this first to set all n00d outputs to 0
  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   aw.pinMode(indexToOuputMap[i], AW9523_LED_MODE);
  //   aw.analogWrite(indexToOuputMap[i], 0);
  // }

  for (int i = 0; i < NUM_NOODS; ++i) {
    ledDriver.pinMode(indexToOuputMap[i], OUTPUT);
    ledDriver.digitalWrite(indexToOuputMap[i], HIGH);
  }

}

void scanDownLoop() {

  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   //aw.analogWrite(indexToOuputMap[i], 0);
  //   ledDriver.digitalWrite(indexToOuputMap[i], HIGH);
  // }

  if (myState != ScannerState::SCAN_DOWN) {
    return;
  }

  if (scannerAnimationTimer > nextFrameTime && scanIndex < NUM_NOODS) {
    ledDriver.digitalWrite(indexToOuputMap[scanIndex], HIGH);
    scanIndex++;
    if (scanIndex >= NUM_NOODS) { setState(ScannerState::SCAN_UPDOWNTRANS); return; }
    ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
    nextFrameTime = timingVector[scanIndex];
    scannerAnimationTimer = 0;
    DEBUG_PRINTLN(scanIndex);
  }
}


void scanUpLoop() {

  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   //aw.analogWrite(indexToOuputMap[i], 0);
  //   ledDriver.digitalWrite(indexToOuputMap[i], HIGH);
  // }

  if (myState != ScannerState::SCAN_UP) {
    return;
  }

  if (scannerAnimationTimer > nextFrameTime && scanIndex >= 0) {
    ledDriver.digitalWrite(indexToOuputMap[scanIndex], HIGH);
    scanIndex--;
    if (scanIndex < 0) {
      if (upDownCounter < upDownCount) {
        upDownCounter++;
        setState(ScannerState::SCAN_DOWN);
        return;
      }
      else {setState(ScannerState::SCAN_UPSHUFFTRANS); upDownCounter = 0; return;}
    }
    ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
    nextFrameTime = timingVector[scanIndex];
    scannerAnimationTimer = 0;
    DEBUG_PRINTLN(scanIndex);
  }
}


void shuffleScanLoop () {
  ledDriver.digitalWrite(indexToOuputMap[scanIndex], HIGH);
  scanIndex = random(0, NUM_NOODS-1);
  ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
}


void deepScanLoop() {
  clearAllOutputs();
  ledDriver.digitalWrite(indexToOuputMap[deepCounter], LOW);
}

void tikTokScanLoop () {
  clearAllOutputs();
  if (tikTokToggle) {
    for (int i = 0; i < halfNoods; ++i) {
      ledDriver.digitalWrite(indexToOuputMap[i], LOW);
    }
  }
  else {
    for (int i = halfNoods; i < NUM_NOODS; ++i) {
        ledDriver.digitalWrite(indexToOuputMap[i], LOW);
    }
  }
  tikTokToggle = !tikTokToggle;
}


void startHandScanDown() {

  clearAllOutputs();
  scanIndex = 0;
  ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
  nextFrameTime = timingVector[scanIndex];
  scannerAnimationTimer = 0;
}


void startHandScanUp() {

  clearAllOutputs();
  scanIndex = NUM_NOODS-1;
  ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
  nextFrameTime = timingVector[scanIndex];
  scannerAnimationTimer = 0;
}


void startShuffleHandScan () {
  clearAllOutputs();
  scanIndex = random(0, NUM_NOODS-1);
  ledDriver.digitalWrite(indexToOuputMap[scanIndex], LOW);
}


void startDeepHandScan() {
  clearAllOutputs();
}


void startTikTokHandScan () {
  clearAllOutputs();
  for (int i = 0; i < halfNoods; ++i) {
    ledDriver.digitalWrite(indexToOuputMap[i], LOW);
  }
  tikTokToggle = false;
}


void stopHandScan() {
  clearAllOutputs();
}


void clearAllOutputs() {
  for (int i = 0; i < NUM_NOODS; ++i) {
    //aw.analogWrite(indexToOuputMap[i], 0);
    ledDriver.digitalWrite(indexToOuputMap[i], HIGH);
  }
}