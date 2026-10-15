// Arduino entry points for the Nour's Zoo rendering milestone.
#include <Arduino.h>
#include "Game.h"

Game game;

// Initialise the game once at boot.
void setup() {
    game.begin();
}

// Keep the loop non-blocking for future input and gameplay.
void loop() {
    game.update(millis());
}
