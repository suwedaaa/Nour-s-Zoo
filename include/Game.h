// Owns zoo/dialogue/menu transitions and frame scheduling.
#pragma once
#include <stdint.h>
#include "Renderer.h"
#include "Input.h"
#include "DialogueData.h"
#include "Conversation.h"
#include "DialogueHistory.h"
#include "Sound.h"
#include "StudyTimer.h"

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
    Conversation conversation_;
    DialogueHistory history_;
    Sound sound_;
    StudyTimer study_;
    bool buttonWasDown_[Input::ButtonCount] = {};
    int selectedSetting_ = 0;
    int selectedChoice_ = 0;
    uint32_t completedTalks_ = 0;
    uint32_t lastFrameMs_ = 0;
    uint32_t sampleStartMs_ = 0;
    uint32_t frameCount_ = 0;
    float fps_ = 0.0f;
    bool ready_ = false;

    void openConversation(DialogueCategory category, uint32_t now);
    void presentNode(uint32_t now);
    void handleInput(uint32_t now);
};
