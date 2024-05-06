#define NUM_NOODS 9

Adafruit_AW9523 ledDriver;

long unsigned timingVectorMillis[] = {210, 47, 24, 8, 16, 16, 17, 47, 210};
long unsigned timeBetweenUpAndDownScan = 300;
int scanIndex = 0;

// LED Outs
int indexToOuput[] = {1, 2, 3, 4, 5, 6, 7, 14, 15};

elapsedMillis scannerAnimationTimer;

long unsigned int nextFrameTime = 0;

bool scanRunning = false;
bool scanDirection = false; // False = scan downward; True = scan upward;

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

void scannerLoop() {

  // for (int i = 0; i < NUM_NOODS; ++i) {
  //   //aw.analogWrite(indexToOuput[i], 0);
  //   ledDriver.digitalWrite(indexToOuput[i], HIGH);
  // }

  if (!scanRunning) {
    return;
  }

  if (scannerAnimationTimer > nextFrameTime && scanIndex < NUM_NOODS) {
    ledDriver.digitalWrite(indexToOuput[scanIndex], HIGH);
    scanIndex++;
    ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
    nextFrameTime = timingVectorMillis[scanIndex];
    scannerAnimationTimer = 0;
    Serial.println(scanIndex);
  }
}


void startHandScan() {

  clearAllOutputs();
  scanIndex = 0;
  ledDriver.digitalWrite(indexToOuput[scanIndex], LOW);
  nextFrameTime = timingVectorMillis[scanIndex];
  scannerAnimationTimer = 0;
  scanRunning = true;

}

void stopHandScan() {

  clearAllOutputs();

  scanRunning = false;
}


void clearAllOutputs() {
  for (int i = 0; i < NUM_NOODS; ++i) {
    //aw.analogWrite(indexToOuput[i], 0);
    ledDriver.digitalWrite(indexToOuput[i], HIGH);
  }
}