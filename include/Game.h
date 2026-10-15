// Owns scene scheduling and measured frame rate.
#pragma once
#include <stdint.h>
#include "Renderer.h"

class Game {
public:
    void begin();
    void update(uint32_t now);

private:
    static constexpr uint32_t FrameIntervalMs = 33;
    Renderer renderer_;
    uint32_t lastFrameMs_ = 0;
    uint32_t sampleStartMs_ = 0;
    uint32_t frameCount_ = 0;
    float fps_ = 0.0f;
    bool ready_ = false;
};
