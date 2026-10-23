// FIRE power adapter: supply detection, brightness, short light-sleep slices and shutdown.
#pragma once
#include "PowerPolicy.h"

class Power {
public:
    void begin();
    void poll(uint32_t now);
    PowerSource source() const { return source_; }
    const char* sourceLabel() const;
    void brightness(IdleState state);
    void sleepSlice();
    void shutdown();
private:
    PowerSource source_ = PowerSource::Unknown;
    uint32_t polledAt_ = 0;
    IdleState brightness_ = IdleState::Awake;
};
