// Plain display data for the milestone's input tester; contains no hardware code.
#pragma once
#include <stdint.h>

struct InputTestView {
    bool down[3] = {};
    uint32_t shortCount[3] = {};
    uint32_t longCount[3] = {};
    char lastEvent[32] = "Last: none";
};
