// Connects gameplay to bounded saves and input/power policies; hardware stays in adapters.
#include <Arduino.h>
#include <esp_system.h>
#include <string.h>
#include <limits.h>
#include "Game.h"

// Reserve framebuffer before save/speaker allocations; sample the boot wipe chord once.
void Game::begin() {
    ready_ = renderer_.begin();
    input_.begin();
    if (!ready_) return;
    sound_.begin();
    power_.begin();
    save_.load();
    const uint32_t now = millis();
    restoreSave(now);
    idle_.begin(now);
    for (int i = 0; i < Input::ButtonCount; ++i)
        buttonWasDown_[i] = input_.isDown(static_cast<ButtonId>(i));
    bootWipe_ = input_.isDown(ButtonId::A) && input_.isDown(ButtonId::C);
    bootHoldAt_ = now;
    if (bootWipe_) screen_ = ZooScreen::BootWipe;
    lastFrameMs_ = sampleStartMs_ = now;
}

// Map saved IDs back to permanent table strings, not table positions.
void Game::restoreSave(uint32_t now) {
    history_ = DialogueHistory{};
    completedTalks_ = save_.data.talks;
    system_.bondXp = save_.data.bondXp;
    system_.debug = save_.data.debug != 0;
    sound_.setVolumeIndex(save_.data.volume);
    for (size_t i = 0; i < save_.data.seenCount; ++i) {
        for (size_t j = 0; j < DialogueData::ScriptCount; ++j) {
            const char* id = DialogueData::Scripts[j].id;
            if (strcmp(id, save_.data.seen[i]) == 0) history_.markCompleted(id);
        }
    }
    history_.restoreLast(nullptr);
    for (size_t j = 0; j < DialogueData::ScriptCount; ++j) {
        if (strcmp(DialogueData::Scripts[j].id, save_.data.lastId) == 0)
            history_.restoreLast(DialogueData::Scripts[j].id);
    }
    study_.stop();
    screen_ = ZooScreen::Zoo;
    if (save_.data.studyActive && save_.data.studyRemainingMs) {
        study_.start(now, save_.data.studyRemainingMs);
        screen_ = ZooScreen::Study;
    }
}

// Atomic meaningful-event snapshot: never called once per frame.
bool Game::checkpoint() {
    static_assert(DialogueHistory::Capacity <= SaveData::Capacity, "Save history must fit");
    auto& data = save_.data;
    data.talks = completedTalks_;
    data.bondXp = system_.bondXp;
    data.volume = sound_.volumeIndex();
    data.debug = system_.debug ? 1 : 0;
    data.studyActive = study_.running() ? 1 : 0;
    data.studyRemainingMs = study_.running() ? study_.remainingMs() : 0;
    data.seenCount = static_cast<uint8_t>(history_.count());
    memset(data.seen, 0, sizeof(data.seen));
    memset(data.lastId, 0, sizeof(data.lastId));
    if (strlen(history_.lastId()) >= SaveData::IdSize) return false;
    strcpy(data.lastId, history_.lastId());
    for (size_t i = 0; i < history_.count(); ++i) {
        if (strlen(history_.idAt(i)) >= SaveData::IdSize) return false;
        strcpy(data.seen[i], history_.idAt(i));
    }
    const bool ok = save_.write();
    if (!ok) Serial.printf("Save blocked/failed: %s\n", save_.label());
    return ok;
}

// Do not discard live progress if the explicit reset could not be stored.
void Game::resetSave(uint32_t now) {
    if (save_.wipe()) {
        conversation_.cancel();
        restoreSave(now);
        selectedAction_ = 0;
    } else screen_ = ZooScreen::SoundSettings;
    idle_.wake(now);
}

// Weighted unlocked tiers, with no-repeat history applied inside each eligible pool.
void Game::openConversation(DialogueCategory category, uint32_t now) {
    const uint16_t bond = static_cast<uint16_t>(
        system_.bondXp / 5 > UINT16_MAX ? UINT16_MAX : system_.bondXp / 5);
    DialogueTier tier = DialogueTier::Normal;
    const uint32_t roll = esp_random() % 100;
    if (category == DialogueCategory::Talk) {
        if (roll < 5 && history_.hasEligible(DialogueData::Scripts, DialogueData::ScriptCount,
                    category, DialogueTier::Secret, bond, completedTalks_))
            tier = DialogueTier::Secret;
        else if (roll < 25 && history_.hasEligible(DialogueData::Scripts, DialogueData::ScriptCount,
                    category, DialogueTier::Rare, bond, completedTalks_))
            tier = DialogueTier::Rare;
    }
    const auto* script = history_.pick(DialogueData::Scripts, DialogueData::ScriptCount,
                                     category, tier, bond, completedTalks_, esp_random());
    if (!conversation_.begin(script)) return;
    Serial.printf("Conversation: %s\n", script->id);
    presentNode(now);
}

// A complete Talk awards one XP and one talk, once, then commits its stable ID.
void Game::presentNode(uint32_t now) {
    if (conversation_.finished()) {
        const auto* script = conversation_.script();
        if (script) {
            history_.markCompleted(script->id);
            if (script->category == DialogueCategory::Talk) {
                if (completedTalks_ < UINT32_MAX) ++completedTalks_;
                if (system_.bondXp < UINT32_MAX) ++system_.bondXp;
            }
            checkpoint();
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

// Menus consume one update at a time; dialogue choices still require a reply.
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
        if ((confirm || previous || next) && dialogue_.confirm(now) && conversation_.advance())
            presentNode(now);
        break;
    case ZooScreen::Actions:
        if (previous || next)
            selectedAction_ = (selectedAction_ + (next ? 1 : 0) - (previous ? 1 : 0) + 3) % 3;
        else if (confirm) {
            if (selectedAction_ == 2) screen_ = ZooScreen::Zoo;
            else if (selectedAction_ == 0) openConversation(DialogueCategory::Talk, now);
            else {
                conversation_.cancel();
                sound_.newSpeech();
                study_.start(now);
                checkpoint();
                screen_ = ZooScreen::Study;
            }
        }
        break;
    case ZooScreen::Choices: {
        const auto* node = conversation_.node();
        if (!node) break;
        if (previous || next)
            selectedChoice_ = (selectedChoice_ + (next ? 1 : 0) - (previous ? 1 : 0) + node->choiceCount) % node->choiceCount;
        else if (confirm && conversation_.choose(selectedChoice_)) presentNode(now);
        break;
    }
    case ZooScreen::Study:
        if (confirm) {
            study_.stop();
            checkpoint();
            screen_ = ZooScreen::Actions;
        } else if (input_.event(ButtonId::B) == PressEvent::Long) {
            selectedSetting_ = 0;
            screen_ = ZooScreen::SoundSettings;
        }
        break;
    case ZooScreen::SoundSettings:
        if (previous || next)
            selectedSetting_ = (selectedSetting_ + (next ? 1 : 0) - (previous ? 1 : 0) + 6) % 6;
        else if (confirm) {
            switch (selectedSetting_) {
            case 0: sound_.cycleVolume(); checkpoint(); break;
            case 1: system_.debug = !system_.debug; checkpoint(); break;
            case 2:
                if (checkpoint()) {
                    screen_ = study_.running() ? ZooScreen::Study : ZooScreen::Zoo;
                    sound_.silence();
                    idle_.sleep(now);
                }
                break;
            case 3: system_.confirmation = 1; screen_ = ZooScreen::ConfirmOff; break;
            case 4: system_.confirmation = 1; screen_ = ZooScreen::ConfirmReset; break;
            case 5: screen_ = study_.running() ? ZooScreen::Study : ZooScreen::Zoo; break;
            }
        }
        break;
    case ZooScreen::ConfirmReset:
    case ZooScreen::ConfirmOff:
        if (previous != next) system_.confirmation = 1 - system_.confirmation;
        else if (confirm) {
            if (system_.confirmation == 1) screen_ = ZooScreen::SoundSettings;
            else if (screen_ == ZooScreen::ConfirmReset) resetSave(now);
            else if (checkpoint()) {
                offPending_ = true;
                offReleasedAt_ = now;
                screen_ = ZooScreen::PoweredOff;
                sound_.silence();
                renderer_.blank();
            } else screen_ = ZooScreen::SoundSettings;
        }
        break;
    default: break;
    }
}

// Sleep never advances gameplay input. Its wake press is consumed through release.
void Game::update(uint32_t now) {
    input_.update();
    if (!ready_) return;
    bool anyDown = false, activity = false;
    for (int i = 0; i < Input::ButtonCount; ++i) {
        const auto button = static_cast<ButtonId>(i);
        anyDown |= input_.isDown(button);
        activity |= input_.isDown(button) || input_.event(button) != PressEvent::None;
    }
    if (offPending_) {
        if (anyDown) offReleasedAt_ = now;
        if (!anyDown && now - offReleasedAt_ >= 100) {
            power_.shutdown();
            // Desktop stays on an OFF screen; hardware deep sleep does not return.
            offPending_ = false;
        }
        return;
    }
    if (screen_ == ZooScreen::PoweredOff) return;
    power_.poll(now);
    const IdleState before = idle_.state();
    const bool consume = idle_.update(now, activity, power_.source());
    if (bootWipe_) {
        if (!input_.isDown(ButtonId::A) || !input_.isDown(ButtonId::C)) {
            bootWipe_ = false;
            screen_ = study_.running() ? ZooScreen::Study : ZooScreen::Zoo;
            idle_.wake(now);
        } else if (now - bootHoldAt_ >= 2000) {
            bootWipe_ = false;
            resetSave(now);
        }
    } else if (!consume) {
        for (int i = 0; i < Input::ButtonCount; ++i) {
            const bool down = input_.isDown(static_cast<ButtonId>(i));
            if (down && !buttonWasDown_[i]) sound_.click();
        }
        handleInput(now);
        if (offPending_) return; // Keep the display black while waiting for release.
        if (screen_ == ZooScreen::Dialogue) {
            const size_t typed = dialogue_.update(now);
            if (dialogue_.speaker() == Speaker::Dodo) sound_.typedCharacters(typed);
        }
    }
    for (int i = 0; i < Input::ButtonCount; ++i)
        buttonWasDown_[i] = input_.isDown(static_cast<ButtonId>(i));
    if (study_.update(now)) {
        checkpoint();
        idle_.wake(now);
        screen_ = ZooScreen::Study;
        sound_.studyComplete();
    }
    if (before != idle_.state() && idle_.state() != IdleState::Awake) {
        checkpoint();
        if (idle_.state() == IdleState::Sleeping) {
            sound_.silence();
            renderer_.blank();
        }
    }
    power_.brightness(idle_.state());
    if (idle_.state() == IdleState::Sleeping) {
        power_.sleepSlice();
        return;
    }
    if (now - lastFrameMs_ < FrameIntervalMs) return;
    lastFrameMs_ = now;
    system_.source = power_.sourceLabel();
    system_.saveStatus = save_.label();
    system_.talks = completedTalks_;
    system_.idle = idle_.state();
    renderer_.drawZoo(screen_, dialogue_, selectedAction_, fps_, conversation_.node(),
                      selectedChoice_, study_, selectedSetting_, sound_.volumeLabel(), system_);
    ++frameCount_;
    const uint32_t completedMs = millis();
    const uint32_t elapsedMs = completedMs - sampleStartMs_;
    if (elapsedMs >= 1000) {
        fps_ = frameCount_ * 1000.0f / elapsedMs;
        frameCount_ = 0;
        sampleStartMs_ = completedMs;
    }
}
