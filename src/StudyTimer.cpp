// Measures elapsed millis without blocking or depending on screen refresh.
#include "StudyTimer.h"

// Starting a new session always resets to the chosen duration.
void StudyTimer::start(uint32_t now, uint32_t durationMs) {
    remainingMs_ = durationMs;
    lastUpdateMs_ = now;
    running_ = true;
    completed_ = false;
}

// Return true exactly once on natural completion; unsigned arithmetic handles rollover.
bool StudyTimer::update(uint32_t now) {
    if (!running_) return false;
    const uint32_t elapsed = now - lastUpdateMs_;
    lastUpdateMs_ = now;
    if (elapsed >= remainingMs_) {
        remainingMs_ = 0;
        running_ = false;
        completed_ = true;
        return true;
    }
    remainingMs_ -= elapsed;
    return false;
}

// Stopping is not a completion and must not trigger the completion sound.
void StudyTimer::stop() {
    running_ = false;
    completed_ = false;
}
