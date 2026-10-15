#include <Arduino.h>
#include <M5Unified.h>

void setup() {
    // Initialise all of the M5Stack FIRE hardware.
    M5.begin();

    // FIRE's screen is 320 × 240 in landscape.
    M5.Display.setRotation(1);

    // Clear the screen.
    M5.Display.fillScreen(TFT_BLACK);

    // Configure our text.
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(3);

    // Draw some text.
    M5.Display.setCursor(30, 100);
    M5.Display.print("NOUR'S ZOO");
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}