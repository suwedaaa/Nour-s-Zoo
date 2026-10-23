// Isolates display hardware and owns the only full-screen RGB565 buffer.
#pragma once
#include <M5Unified.h>
#include "Dialogue.h"
#include "DialogueData.h"
#include "StudyTimer.h"
#include "SystemView.h"

enum class ZooScreen : uint8_t { Zoo, Dialogue, Actions, Choices, Study, SoundSettings,
                                 ConfirmReset, ConfirmOff, BootWipe, PoweredOff };

class Renderer {
public:
    bool begin();
    void blank();
    void drawZoo(ZooScreen screen, const Dialogue& dialogue, int selectedAction, float fps,
                 const DialogueNode* choiceNode, int selectedChoice,
                 const StudyTimer& study, int selectedSetting, const char* volume,
                 const SystemView& system);

private:
    static constexpr int ScreenWidth = 320;
    static constexpr int ScreenHeight = 240;
    static constexpr int AnimalCanvasSize = 24;
    static constexpr int AnimalScale = 2;
    M5Canvas frame_{&M5.Display};
    bool usingPsram_ = false;

    void drawScene();
    void drawDodo(int left, int top, int scale);
    void drawDialogue(const Dialogue& dialogue);
    void drawActions(int selectedAction);
    void drawChoices(const DialogueNode& node, int selectedChoice);
    void drawStudy(const StudyTimer& study);
    void drawSoundSettings(int selectedSetting, const char* volume, const SystemView& system);
    void drawConfirmation(bool reset, int selection);
};

