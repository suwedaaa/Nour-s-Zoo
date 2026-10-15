#include <Arduino.h>
#include <M5Unified.h>

#include "background1.h"

void setup() {
    // Initialise all of the M5Stack FIRE hardware.
    M5.begin();

    // FIRE's screen is 320 × 240 in landscape.
    M5.Display.setRotation(1);

    // Clear the screen.
    M5.Display.fillScreen(TFT_BLACK);

    // Draw the embedded PNG at coordinate (0, 0).
    // sizeof gives the PNG's size in bytes.
    M5.Display.drawPng(
        background1_png,
        background1_png_len,
        0,
        0
    );
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}