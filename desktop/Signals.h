#pragma once
#include <cstdint>
// Desktop-only output hooks for future hardware adapters. Tests are labelled in the UI.
void simulatorLed(int index, uint8_t red, uint8_t green, uint8_t blue);
void simulatorVibration(uint32_t durationMs);
