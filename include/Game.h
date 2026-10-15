// Owns zoo/dialogue/menu transitions and frame scheduling.
#pragma once
#include <stdint.h>
#include "Renderer.h"
#include "Input.h"
#include "DialogueData.h"

class Game {
public:
    void begin();
    void update(uint32_t now);

private:
    static constexpr uint32_t FrameIntervalMs = 33;
    Renderer renderer_;
    Input input_;
    Dialogue dialogue_;
    ZooScreen screen_ = ZooScreen::Zoo;
    int selectedAction_ = 0;
    bool greetingSeen_[DialogueData::GreetingCount] = {};
    uint32_t lastFrameMs_ = 0;
    uint32_t sampleStartMs_ = 0;
    uint32_t frameCount_ = 0;
    float fps_ = 0.0f;
    bool ready_ = false;

    void openGreeting(uint32_t now);
    void handleInput(uint32_t now);
};
