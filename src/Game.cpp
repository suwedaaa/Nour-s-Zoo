// Schedules complete frames and measures completed frames per second.
#include <Arduino.h>
#include "Game.h"

// Allocate the renderer before starting the frame clock.
void Game::begin() {
    ready_ = renderer_.begin();
    lastFrameMs_ = sampleStartMs_ = millis();
}

// Unsigned subtraction remains correct when millis() rolls over.
void Game::update(uint32_t now) {
    if (!ready_ || now - lastFrameMs_ < FrameIntervalMs) {
        return;
    }
    lastFrameMs_ = now;
    renderer_.drawZoo(fps_);
    ++frameCount_;

    // Include display transfer time in actual throughput.
    const uint32_t completedMs = millis();
    const uint32_t elapsedMs = completedMs - sampleStartMs_;
    if (elapsedMs >= 1000) {
        fps_ = frameCount_ * 1000.0f / elapsedMs;
        frameCount_ = 0;
        sampleStartMs_ = completedMs;
    }
}
