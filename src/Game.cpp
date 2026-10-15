// Schedules complete frames and measures completed frames per second.
#include <Arduino.h>
#include <stdio.h>
#include "Game.h"

// Allocate the renderer before starting the frame clock.
void Game::begin() {
    ready_ = renderer_.begin();
    input_.begin();
    lastFrameMs_ = sampleStartMs_ = millis();
}

// Unsigned subtraction remains correct when millis() rolls over.
void Game::update(uint32_t now) {
    input_.update();
    // Consume events every loop, before the display's frame-rate limiter.
    for (int index = 0; index < Input::ButtonCount; ++index) {
        const auto button = static_cast<ButtonId>(index);
        inputTest_.down[index] = input_.isDown(button);
        const PressEvent event = input_.event(button);
        if (event == PressEvent::None) {
            continue;
        }
        if (event == PressEvent::Short) {
            ++inputTest_.shortCount[index];
        } else {
            ++inputTest_.longCount[index];
        }
        snprintf(inputTest_.lastEvent, sizeof(inputTest_.lastEvent),
                 "Last: %c %s", 'A' + index,
                 event == PressEvent::Short ? "SHORT" : "LONG");
        Serial.println(inputTest_.lastEvent);
    }
    if (!ready_ || now - lastFrameMs_ < FrameIntervalMs) {
        return;
    }
    lastFrameMs_ = now;
    renderer_.drawZoo(fps_, inputTest_);
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
