// Native tests for exact countdown boundaries, stopping, restart and millis rollover.
#include <assert.h>
#include <stdio.h>
#include "StudyTimer.h"

int main() {
    StudyTimer timer;
    assert(!timer.running() && !timer.completed());
    timer.start(100);
    assert(timer.remainingSeconds() == 1500);
    assert(!timer.update(101));
    assert(timer.remainingSeconds() == 1500);
    assert(!timer.update(1100));
    assert(timer.remainingSeconds() == 1499);
    // A long gap in rendering must not pause or extend the timer.
    assert(!timer.update(100 + StudyTimer::DefaultDurationMs - 1));
    assert(timer.remainingMs() == 1 && timer.remainingSeconds() == 1);
    assert(timer.update(100 + StudyTimer::DefaultDurationMs));
    assert(!timer.running() && timer.completed() && timer.remainingSeconds() == 0);
    assert(!timer.update(100 + StudyTimer::DefaultDurationMs + 1000));

    timer.start(0, 1000);
    timer.update(400);
    timer.stop();
    assert(!timer.completed() && !timer.update(2000));
    assert(timer.remainingMs() == 600);
    timer.start(2000);
    assert(timer.remainingSeconds() == 1500 && timer.running());

    const uint32_t start = 0xFFFFFF00u;
    timer.start(start, 1000);
    assert(!timer.update(start + 500u));
    assert(timer.remainingMs() == 500);
    assert(timer.update(start + 2000u));
    assert(timer.remainingMs() == 0);
    timer.start(0, 0);
    assert(timer.update(0));
    assert(!timer.update(1));
    puts("PASS: 25-minute default, second rounding, long update gaps, exact completion, stop/restart, rollover and one-shot finish.");
}
