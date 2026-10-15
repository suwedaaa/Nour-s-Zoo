// Connects input events to dialogue/menu state without putting hardware in dialogue.
#include <Arduino.h>
#include <esp_system.h>
#include "Game.h"

// Initialise display hardware before the input adapter samples buttons.
void Game::begin() {
    ready_ = renderer_.begin();
    input_.begin();
    lastFrameMs_ = sampleStartMs_ = millis();
}

// Choose among eligible unseen greetings; reset only when the pool is exhausted.
void Game::openGreeting(uint32_t now) {
    int eligible[DialogueData::GreetingCount];
    int count = 0;
    // Bond progression and persistent history belong to the save milestone.
    constexpr uint16_t CurrentBond = 0;
    for (int pass = 0; pass < 2 && count == 0; ++pass) {
        for (int index = 0; index < DialogueData::GreetingCount; ++index) {
            const auto& line = DialogueData::Greetings[index];
            if (line.tier == DialogueTier::Normal && line.minimumBond <= CurrentBond &&
                !greetingSeen_[index]) {
                eligible[count++] = index;
            }
        }
        if (count == 0) {
            for (int index = 0; index < DialogueData::GreetingCount; ++index) greetingSeen_[index] = false;
        }
    }
    if (count == 0) return;
    const int chosen = eligible[esp_random() % count];
    greetingSeen_[chosen] = true;
    selectedAction_ = 0;
    dialogue_.begin(Speaker::Dodo, DialogueData::Greetings[chosen].text, now);
    screen_ = ZooScreen::Dialogue;
}

// One input update operates on one screen, preventing B from also confirming a menu.
void Game::handleInput(uint32_t now) {
    const bool confirm = input_.event(ButtonId::B) == PressEvent::Short;
    switch (screen_) {
    case ZooScreen::Zoo:
        if (confirm) openGreeting(now);
        break;
    case ZooScreen::Dialogue:
        // Treat simultaneous taps as one advance, never one advance per button.
        if ((confirm || input_.event(ButtonId::A) == PressEvent::Short ||
             input_.event(ButtonId::C) == PressEvent::Short) && dialogue_.confirm(now)) {
            screen_ = ZooScreen::Actions;
        }
        break;
    case ZooScreen::Actions: {
        const bool previous = input_.event(ButtonId::A) == PressEvent::Short;
        const bool next = input_.event(ButtonId::C) == PressEvent::Short;
        // Simultaneous opposite directions cancel. Navigation takes priority
        // over confirmation so a combined press cannot choose an unseen item.
        if (previous || next) {
            selectedAction_ = (selectedAction_ + (next ? 1 : 0) - (previous ? 1 : 0) + 3) % 3;
        } else if (confirm) {
            if (selectedAction_ == 2) {
                screen_ = ZooScreen::Zoo;
            } else {
                const bool talk = selectedAction_ == 0;
                dialogue_.begin(talk ? Speaker::Dodo : Speaker::Player,
                                talk ? DialogueData::Talk.text : DialogueData::StudyPreview.text, now);
                screen_ = ZooScreen::Dialogue;
            }
        }
        break;
    }
    }
}

// Poll input continuously; compose/push complete frames at approximately 30 FPS.
void Game::update(uint32_t now) {
    input_.update();
    if (!ready_) return;
    // Handle input before ticking text: a tap on an incomplete page reveals it,
    // even if this update's elapsed time would otherwise finish typing.
    handleInput(now);
    if (screen_ == ZooScreen::Dialogue) dialogue_.update(now);
    if (now - lastFrameMs_ < FrameIntervalMs) return;
    lastFrameMs_ = now;
    renderer_.drawZoo(screen_, dialogue_, selectedAction_, fps_);
    ++frameCount_;
    const uint32_t completedMs = millis();
    const uint32_t elapsedMs = completedMs - sampleStartMs_;
    if (elapsedMs >= 1000) {
        fps_ = frameCount_ * 1000.0f / elapsedMs;
        frameCount_ = 0;
        sampleStartMs_ = completedMs;
    }
}
