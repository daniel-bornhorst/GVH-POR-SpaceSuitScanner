#define NUM_NOODS 9

Adafruit_AW9523 ledDriver;

long unsigned timingVector[] = {210, 48, 25, 9, 16, 17, 17, 48, 210};  // In Milliseconds
long unsigned timeBetweenUpAndDownScan = 300;
int scanIndex = 0;

// LED Outs
int indexToOuput[] = {1, 2, 3, 4, 5, 6, 7, 14, 15};

elapsedMillis scannerAnimationTimer;

long unsigned int nextFrameTime = 0;

void scannerSetup() {

  if (! ledDriver.begin(0x58)) {
    Serial.println("LED Driver not found? Check wiring!");
    while (1) delay(10);  // halt forever
  }
  Serial.println("LED Driver found!");


  // Do this first to set all n00d outputs to 0
  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   aw.pinMode(indexToOuput[i], AW9523_LED_MODE);
  //   aw.analogWrite(indexToOuput[i], 0);
  // }

  for (int i = 0; i < NUM_NOODS; ++i) {
    ledDriver.pinMode(indexToOuput[i], OUTPUT);
    ledDriver.digitalWrite(indexToOuput[i], HIGH);
  }

}

void scanDownLoop() {

  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   //aw.analogWrite(indexToOuput[i], 0);
  //   ledDriver.digitalWrite(indexToOuput[i], HIGH);
  // }

  if (myState != ScannerState::SCAN_DOWN) {
    return;
  }

  if (scannerAnimationTimer > nextFrameTime && scanIndex < NUM_NOODS) {
    ledDriver.digitalWrite(indexToOuput[scanIndex], HIGH);
    scanIndex++;
    if (scanIndex >= NUM_NOODS) { setState(ScannerState::SCAN_UPDOWNTRANS); return; }
    ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
    nextFrameTime = timingVector[scanIndex];
    scannerAnimationTimer = 0;
    DEBUG_PRINTLN(scanIndex);
  }
}


void scanUpLoop() {

  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   //aw.analogWrite(indexToOuput[i], 0);
  //   ledDriver.digitalWrite(indexToOuput[i], HIGH);
  // }

  if (myState != ScannerState::SCAN_UP) {
    return;
  }

  if (scannerAnimationTimer > nextFrameTime && scanIndex >= 0) {
    ledDriver.digitalWrite(indexToOuput[scanIndex], HIGH);
    scanIndex--;
    if (scanIndex < 0) { setState(ScannerState::SHUFFLE_SCAN); return; }
    ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
    nextFrameTime = timingVector[scanIndex];
    scannerAnimationTimer = 0;
    DEBUG_PRINTLN(scanIndex);
  }
}


void startHandScanDown() {

  clearAllOutputs();
  scanIndex = 0;
  ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
  nextFrameTime = timingVector[scanIndex];
  scannerAnimationTimer = 0;

}


void startHandScanUp() {

  clearAllOutputs();
  scanIndex = NUM_NOODS-1;
  ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
  nextFrameTime = timingVector[scanIndex];
  scannerAnimationTimer = 0;

}

void stopHandScan() {

  clearAllOutputs();
}


void clearAllOutputs() {
  for (int i = 0; i < NUM_NOODS; ++i) {
    //aw.analogWrite(indexToOuput[i], 0);
    ledDriver.digitalWrite(indexToOuput[i], HIGH);
  }
}