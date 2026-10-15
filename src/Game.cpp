// Connects input to data-driven dialogue, narration, choices and completion history.
#include <Arduino.h>
#include <esp_system.h>
#include "Game.h"

// Initialise the display (including early framebuffer allocation) before input.
void Game::begin() {
    ready_ = renderer_.begin();
    input_.begin();
    if (ready_) sound_.begin();
    for (int index = 0; index < Input::ButtonCount; ++index) {
        buttonWasDown_[index] = input_.isDown(static_cast<ButtonId>(index));
    }
    if (DialogueData::ScriptCount > DialogueHistory::Capacity) {
        Serial.println("Dialogue table exceeds history capacity.");
        ready_ = false;
    }
    lastFrameMs_ = sampleStartMs_ = millis();
}

// Pick a tier only if unlocked: normal 75%, rare 20%, secret 5% once all unlock.
void Game::openConversation(DialogueCategory category, uint32_t now) {
    constexpr uint16_t CurrentBond = 0; // Bond progression comes with the save milestone.
    DialogueTier tier = DialogueTier::Normal;
    const uint32_t roll = esp_random() % 100;
    if (category == DialogueCategory::Talk) {
        if (roll < 5 && history_.hasEligible(DialogueData::Scripts, DialogueData::ScriptCount,
                    category, DialogueTier::Secret, CurrentBond, completedTalks_)) {
            tier = DialogueTier::Secret;
        } else if (roll < 25 && history_.hasEligible(DialogueData::Scripts, DialogueData::ScriptCount,
                    category, DialogueTier::Rare, CurrentBond, completedTalks_)) {
            tier = DialogueTier::Rare;
        }
    }
    const auto* script = history_.pick(DialogueData::Scripts, DialogueData::ScriptCount,
                                     category, tier, CurrentBond, completedTalks_, esp_random());
    if (!conversation_.begin(script)) return;
    Serial.printf("Conversation: %s (completed talks: %lu)\n",
                  script->id, static_cast<unsigned long>(completedTalks_));
    presentNode(now);
}

// Present the next line or choice, or record the finished conversation exactly once.
void Game::presentNode(uint32_t now) {
    if (conversation_.finished()) {
        const auto* script = conversation_.script();
        if (script) {
            if (!history_.markCompleted(script->id)) {
                Serial.println("Dialogue history is full.");
            }
            if (script->category == DialogueCategory::Talk) ++completedTalks_;
            Serial.printf("Completed: %s\n", script->id);
        }
        conversation_.cancel();
        screen_ = ZooScreen::Actions;
        return;
    }
    const auto* node = conversation_.node();
    if (!node) return;
    if (node->choiceCount) {
        selectedChoice_ = 0;
        screen_ = ZooScreen::Choices;
    } else {
        dialogue_.begin(node->speaker, node->text, now);
        sound_.newSpeech();
        screen_ = ZooScreen::Dialogue;
    }
}

// Each input update acts on one screen, so a reveal cannot also select a reply.
void Game::handleInput(uint32_t now) {
    const bool confirm = input_.event(ButtonId::B) == PressEvent::Short;
    const bool previous = input_.event(ButtonId::A) == PressEvent::Short;
    const bool next = input_.event(ButtonId::C) == PressEvent::Short;
    switch (screen_) {
    case ZooScreen::Zoo:
        if (input_.event(ButtonId::B) == PressEvent::Long) {
            selectedSetting_ = 0;
            screen_ = ZooScreen::SoundSettings;
        } else if (confirm) {
            selectedAction_ = 0;
            openConversation(DialogueCategory::Greeting, now);
        }
        break;
    case ZooScreen::Dialogue:
        if ((confirm || previous || next) && dialogue_.confirm(now)) {
            if (conversation_.script()) {
                if (conversation_.advance()) presentNode(now);
            } else {
                screen_ = ZooScreen::Actions;
            }
        }
        break;
    case ZooScreen::Actions:
        if (previous || next) {
            selectedAction_ = (selectedAction_ + (next ? 1 : 0) - (previous ? 1 : 0) + 3) % 3;
        } else if (confirm) {
            if (selectedAction_ == 2) {
                screen_ = ZooScreen::Zoo;
            } else if (selectedAction_ == 0) {
                openConversation(DialogueCategory::Talk, now);
            } else {
                conversation_.cancel();
                sound_.newSpeech();
                study_.start(now);
                screen_ = ZooScreen::Study;
            }
        }
        break;
    case ZooScreen::Choices: {
        const auto* node = conversation_.node();
        if (!node) break;
        const int itemCount = node->choiceCount;
        if (previous || next) {
            selectedChoice_ = (selectedChoice_ + (next ? 1 : 0) - (previous ? 1 : 0) + itemCount) % itemCount;
        } else if (confirm) {
            if (conversation_.choose(selectedChoice_)) {
                presentNode(now);
            }
        }
        break;
    }
    case ZooScreen::Study:
        if (confirm) {
            study_.stop();
            screen_ = ZooScreen::Actions;
        }
        break;
    case ZooScreen::SoundSettings:
        if (previous || next) {
            selectedSetting_ = (selectedSetting_ + (next ? 1 : 0) - (previous ? 1 : 0) + 2) % 2;
        } else if (confirm) {
            if (selectedSetting_ == 0) sound_.cycleVolume();
            else screen_ = ZooScreen::Zoo;
        }
        break;
    }
}

// Poll continuously and render complete frames at approximately 30 FPS.
void Game::update(uint32_t now) {
    input_.update();
    if (!ready_) return;
    for (int index = 0; index < Input::ButtonCount; ++index) {
        const bool down = input_.isDown(static_cast<ButtonId>(index));
        if (down && !buttonWasDown_[index]) sound_.click();
        buttonWasDown_[index] = down;
    }
    handleInput(now);
    if (screen_ == ZooScreen::Dialogue) {
        const size_t typed = dialogue_.update(now);
        if (dialogue_.speaker() == Speaker::Dodo) sound_.typedCharacters(typed);
    }
    // Timer updates independently of drawing, ready for later screen dimming.
    if (study_.update(now)) sound_.studyComplete();
    if (now - lastFrameMs_ < FrameIntervalMs) return;
    lastFrameMs_ = now;
    renderer_.drawZoo(screen_, dialogue_, selectedAction_, fps_,
                      conversation_.node(), selectedChoice_, study_, selectedSetting_, sound_.volumeLabel());
    ++frameCount_;
    const uint32_t completedMs = millis();
    const uint32_t elapsedMs = completedMs - sampleStartMs_;
    if (elapsedMs >= 1000) {
        fps_ = frameCount_ * 1000.0f / elapsedMs;
        frameCount_ = 0;
        sampleStartMs_ = completedMs;
    }
}
