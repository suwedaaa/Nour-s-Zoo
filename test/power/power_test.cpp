// Checks idle thresholds, supply decisions, consumed wake gestures, and timer continuity.
#include "PowerPolicy.h"
#include "StudyTimer.h"
#include <assert.h>
#include <stdio.h>
#include <initializer_list>

int main() {
    PowerPolicy policy;
    policy.begin(0);
    assert(!policy.update(59999, false, PowerSource::Battery));
    assert(policy.update(60000, false, PowerSource::Battery));
    assert(policy.state() == IdleState::Dim);
    assert(policy.update(300000, false, PowerSource::Battery));
    assert(policy.state() == IdleState::Sleeping);
    assert(policy.update(300001, true, PowerSource::Battery));
    assert(policy.state() == IdleState::Awake);
    assert(policy.update(301000, true, PowerSource::Battery)); // Long wake hold.
    assert(policy.update(301001, true, PowerSource::Battery)); // Release event.
    assert(policy.update(301050, false, PowerSource::Battery));
    assert(policy.update(301051, false, PowerSource::Battery));
    assert(!policy.update(301052, false, PowerSource::Battery));
    assert(!policy.update(301100, true, PowerSource::Battery)); // Fresh press acts.
    for (auto source : {PowerSource::External, PowerSource::Unknown}) {
        policy.begin(0);
        assert(policy.update(600000, false, source));
        assert(policy.state() == IdleState::Dim); // Never auto-sleep on uncertain supply.
        policy.sleep(600001);
        assert(policy.update(600002, false, source));
        assert(policy.state() == IdleState::Sleeping); // Explicit sleep is allowed.
    }
    policy.begin(UINT32_MAX - 999);
    assert(policy.update(59000, false, PowerSource::Battery));
    assert(policy.state() == IdleState::Dim);
    assert(policy.update(299000, false, PowerSource::Battery));
    assert(policy.state() == IdleState::Sleeping);
    StudyTimer study;
    study.start(0);
    assert(!study.update(300000));
    assert(study.remainingMs() == StudyTimer::DefaultDurationMs - 300000);
    assert(study.update(StudyTimer::DefaultDurationMs));
    assert(!study.update(StudyTimer::DefaultDurationMs + 1000));
    puts("Power tests passed: dim/sleep, USB/unknown, wake release, rollover, Study continuity.");
}
