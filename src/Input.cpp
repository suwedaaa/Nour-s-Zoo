// Samples M5Unified buttons; gameplay never needs to read hardware directly.
#include <Arduino.h>
#include <M5Unified.h>
#include "Input.h"

// M5.begin() must have run before this initial sample.
void Input::begin() {
    // Our own state machine handles debounce consistently, including slow frames.
    M5.BtnA.setDebounceThresh(0);
    M5.BtnB.setDebounceThresh(0);
    M5.BtnC.setDebounceThresh(0);
    M5.update();
    const uint32_t now = millis();
    buttons_[0].begin(M5.BtnA.isPressed(), now);
    buttons_[1].begin(M5.BtnB.isPressed(), now);
    buttons_[2].begin(M5.BtnC.isPressed(), now);
}

// Events last one update; Game consumes them immediately, even between frames.
void Input::update() {
    M5.update();
    const uint32_t now = millis();
    events_[0] = buttons_[0].update(M5.BtnA.isPressed(), now);
    events_[1] = buttons_[1].update(M5.BtnB.isPressed(), now);
    events_[2] = buttons_[2].update(M5.BtnC.isPressed(), now);
}

// Return only this update's classified event.
PressEvent Input::event(ButtonId button) const {
    return events_[static_cast<int>(button)];
}

// Expose debounced held state for UI and future button combinations.
bool Input::isDown(ButtonId button) const {
    return buttons_[static_cast<int>(button)].isDown();
}
