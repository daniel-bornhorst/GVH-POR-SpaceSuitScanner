
Adafruit_IS31FL3731 ledmatrix = Adafruit_IS31FL3731();

uint16_t pixels[16][9];
uint8_t x_cord = 0;
uint8_t y_cord = 0;

elapsedMillis matrixAnimationTimer;

// long unsigned int matrixRefreshRate = 30;

// int matrixMode = 2;

void matrixSetup() {
  if (! ledmatrix.begin(0x74, &Wire1)) {
    Serial.println("IS31 not found");
    while (1);
  }
  Serial.println("IS31 found!");
}

void matrixLoop() {

  if (matrixAnimationTimer > matrixRefreshRate) {

    if (matrixMode == 0) {

    }

    if (matrixMode == 1) {
      randomizePixelArray(0, 10);
      for (uint8_t y = 0; y < 9; y++) {
        for (uint8_t x = 0; x < 16; x++) {
          ledmatrix.drawPixel(x, y, pixels[x][y]);
        }
      }
    }

    if (matrixMode == 2) {

      ledmatrix.drawPixel(x_cord, y_cord, 255);
      //delay(freq);
      ledmatrix.clear();

      x_cord++;

      if (x_cord >= 16) {
        x_cord = 0;
        y_cord++;

        if (y_cord >= 9) {
          y_cord = 0;
        }
      }
    }

    if (matrixMode == 3) {
      updateMatrixPixels(maxtrixRandomizer);
      for (uint8_t y = 0; y < 9; y++) {
        for (uint8_t x = 0; x < 16; x++) {
          ledmatrix.drawPixel(x, y, pixels[x][y]);
        }
      }
    }
    matrixAnimationTimer = 0; 
  }
}


void randomizePixelArray(int min, int max) {
  for (uint8_t y = 0; y < 9; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      pixels[x][y] = random(min, max);
    }
  }
}


// 60, 20, 5, 1
#define val1 60
#define val2 18
#define val3 5
#define val4 1
void updateMatrixPixels(int frequency) {
  // Create and zero out a 2d array-
  uint16_t newPixels[16][9];
  for (uint8_t y = 0; y < 9; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      newPixels[x][y] = 0;
    }
  }

  uint8_t x = 0;
  uint8_t y = 0;
  int rand = 0;
  for (y = 0; y < 9; y++) {
    x = 0;
    
    // randomly start new particals
    int rand = random(0,100);
    if (rand != 0 && rand != frequency && rand % frequency == 0 && pixels[x][y] == 0) {
      newPixels[x][y] = val1;
      // DEBUG_PRINT("HIT!   randVal = ")
      // DEBUG_PRINTLN(rand)
    }
    else if (pixels[x][y] == val1) {
      newPixels[x][y] = val2;
    }
    else if (pixels[x][y] == val2) {
      newPixels[x][y] = val3;
    }
    else if (pixels[x][y] == val3) {
      newPixels[x][y] = val4;
    }

    for (; x < 16; x++) {
      if (pixels[x][y] != 0 && x < 15) {
        newPixels[x+1][y] = pixels[x][y];
      }
    }
  }

  //pixels = newPixels;
  memcpy(pixels, newPixels, sizeof(newPixels));
}