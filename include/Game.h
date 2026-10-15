// Owns scene scheduling and measured frame rate.
#pragma once
#include <stdint.h>
#include "Renderer.h"
#include "Input.h"

class Game {
public:
    void begin();
    void update(uint32_t now);

private:
    static constexpr uint32_t FrameIntervalMs = 33;
    Renderer renderer_;
    Input input_;
    InputTestView inputTest_;
    uint32_t lastFrameMs_ = 0;
    uint32_t sampleStartMs_ = 0;
    uint32_t frameCount_ = 0;
    float fps_ = 0.0f;
    bool ready_ = false;
};
