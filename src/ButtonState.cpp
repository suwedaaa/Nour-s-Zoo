// Implements non-blocking debounce and one event per completed press or hold.
#include "ButtonState.h"

// A button held at startup must be released before it can trigger an action.
void ButtonState::begin(bool pressed, uint32_t now) {
    candidateDown_ = stableDown_ = pressed;
    candidateSince_ = pressedSince_ = now;
    longSent_ = false;
    armed_ = !pressed;
}

// Require 25 ms of consistent samples for both press and release transitions.
PressEvent ButtonState::update(bool pressed, uint32_t now) {
    if (pressed != candidateDown_) {
        candidateDown_ = pressed;
        candidateSince_ = now;
    }

    if (candidateDown_ != stableDown_ &&
        now - candidateSince_ >= DebounceMs) {
        stableDown_ = candidateDown_;
        if (stableDown_) {
            pressedSince_ = candidateSince_;
            longSent_ = false;
        } else {
            // Use the start of the stable release, excluding release debounce.
            // This also classifies a long hold if polling skipped its threshold.
            const uint32_t heldMs = candidateSince_ - pressedSince_;
            const bool shouldSend = armed_ && !longSent_;
            armed_ = true;
            if (shouldSend) {
                return heldMs >= LongPressMs ? PressEvent::Long : PressEvent::Short;
            }
        }
    }

    // Do not promote a short press to long while its release is debouncing.
    // Once long fires, releasing this press can never produce short.
    if (armed_ && stableDown_ && candidateDown_ && !longSent_ &&
        now - pressedSince_ >= LongPressMs) {
        longSent_ = true;
        return PressEvent::Long;
    }
    return PressEvent::None;
}
