// Native tests for button timing, bouncing contacts and mutually exclusive events.
#include <assert.h>
#include <stdio.h>
#include "ButtonState.h"

// Feed samples with explicit timestamps; no real clock or device is required.
int main() {
    ButtonState button;
    button.begin(false, 0);
    assert(button.update(true, 100) == PressEvent::None);
    assert(button.update(false, 110) == PressEvent::None);
    assert(button.update(true, 120) == PressEvent::None);
    assert(button.update(true, 144) == PressEvent::None);
    assert(!button.isDown());
    assert(button.update(true, 145) == PressEvent::None);
    assert(button.isDown());
    assert(button.update(false, 200) == PressEvent::None);
    assert(button.update(true, 210) == PressEvent::None);
    assert(button.update(false, 220) == PressEvent::None);
    assert(button.update(false, 245) == PressEvent::Short);
    assert(button.update(false, 300) == PressEvent::None);
    assert(!button.isDown());

    // Long is sent at 700 ms, exactly once, with no short on release.
    button.begin(false, 0);
    button.update(true, 100);
    button.update(true, 125);
    assert(button.update(true, 799) == PressEvent::None);
    assert(button.update(true, 800) == PressEvent::Long);
    assert(button.update(true, 5000) == PressEvent::None);
    button.update(false, 5010);
    assert(button.update(false, 5035) == PressEvent::None);
    // A subsequent short press still works.
    button.update(true, 5100);
    button.update(true, 5125);
    button.update(false, 5200);
    assert(button.update(false, 5225) == PressEvent::Short);

    // Release at 699 ms stays short even though debounce finishes after 700.
    button.begin(false, 0);
    button.update(true, 100);
    button.update(true, 125);
    button.update(false, 799);
    assert(button.update(false, 810) == PressEvent::None);
    assert(button.update(false, 824) == PressEvent::Short);

    // A slow frame may skip the hold threshold: release must still report long.
    button.begin(false, 0);
    button.update(true, 100);
    button.update(true, 125);
    button.update(false, 850);
    assert(button.update(false, 875) == PressEvent::Long);
    assert(button.update(false, 900) == PressEvent::None);

    // Ignore buttons held at boot until a debounced release.
    button.begin(true, 0);
    assert(button.update(true, 2000) == PressEvent::None);
    button.update(false, 2100);
    assert(button.update(false, 2125) == PressEvent::None);
    button.update(true, 2200);
    button.update(true, 2225);
    button.update(false, 2300);
    assert(button.update(false, 2325) == PressEvent::Short);

    // Millisecond rollover must not change hold duration.
    const uint32_t start = 0xFFFFFF00u;
    button.begin(false, start - 1);
    button.update(true, start);
    button.update(true, start + 25u);
    assert(button.update(true, start + 699u) == PressEvent::None);
    assert(button.update(true, start + 700u) == PressEvent::Long);
    button.update(false, start + 800u);
    assert(button.update(false, start + 825u) == PressEvent::None);

    // All three buttons track simultaneous holds independently.
    ButtonState buttons[3];
    for (auto& item : buttons) {
        item.begin(false, 0);
        item.update(true, 100);
        item.update(true, 125);
    }
    for (auto& item : buttons) {
        assert(item.update(true, 800) == PressEvent::Long);
        item.update(false, 900);
        assert(item.update(false, 925) == PressEvent::None);
    }
    puts("PASS: debounce, short/long exclusion, repeat suppression, boundary timing, boot hold, rollover, simultaneous buttons.");
    return 0;
}
