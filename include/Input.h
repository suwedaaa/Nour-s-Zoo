// Isolates M5 button hardware from reusable per-button press classification.
#pragma once
#include "ButtonState.h"

enum class ButtonId : uint8_t { A, B, C };

class Input {
public:
    static constexpr int ButtonCount = 3;
    void begin();
    void update();
    PressEvent event(ButtonId button) const;
    bool isDown(ButtonId button) const;

private:
    ButtonState buttons_[ButtonCount];
    PressEvent events_[ButtonCount] = {};
};
