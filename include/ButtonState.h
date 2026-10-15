// Tracks one button's stable state and emits mutually exclusive press events.
#pragma once
#include <stdint.h>

enum class PressEvent : uint8_t { None, Short, Long };

class ButtonState {
public:
    static constexpr uint32_t DebounceMs = 25;
    static constexpr uint32_t LongPressMs = 700;

    void begin(bool pressed, uint32_t now);
    PressEvent update(bool pressed, uint32_t now);
    bool isDown() const { return stableDown_; }

private:
    bool candidateDown_ = false;
    bool stableDown_ = false;
    bool longSent_ = false;
    bool armed_ = true;
    uint32_t candidateSince_ = 0;
    uint32_t pressedSince_ = 0;
};
