// Plain system/debug view data; renderer never reads flash or power hardware.
#pragma once
#include <stdint.h>
#include "PowerPolicy.h"

struct SystemView {
    bool debug = false;
    const char* source = "Unknown";
    const char* saveStatus = "New save";
    uint32_t bondXp = 0;
    uint32_t talks = 0;
    int confirmation = 1;
    IdleState idle = IdleState::Awake;
};
