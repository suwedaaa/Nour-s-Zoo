// A hardware-independent countdown that keeps time independently of rendering.
#pragma once
#include <stdint.h>

class StudyTimer {
public:
    static constexpr uint32_t DefaultDurationMs = 25UL * 60UL * 1000UL;
    void start(uint32_t now, uint32_t durationMs = DefaultDurationMs);
    bool update(uint32_t now);
    void stop();
    bool running() const { return running_; }
    bool completed() const { return completed_; }
    uint32_t remainingMs() const { return remainingMs_; }
    uint32_t remainingSeconds() const {
        return remainingMs_ / 1000 + (remainingMs_ % 1000 != 0 ? 1 : 0);
    }

private:
    uint32_t remainingMs_ = 0;
    uint32_t lastUpdateMs_ = 0;
    bool running_ = false;
    bool completed_ = false;
};
