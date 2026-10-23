// Keeps inactivity and input-consumption rules independent of the power hardware.
#include "PowerPolicy.h"

void PowerPolicy::begin(uint32_t now) {
    lastActivity_ = quietSince_ = now;
    swallowing_ = false;
    state_ = IdleState::Awake;
}
void PowerPolicy::wake(uint32_t now) {
    begin(now);
    swallowing_ = true;
}
void PowerPolicy::sleep(uint32_t now) {
    lastActivity_ = now;
    state_ = IdleState::Sleeping;
}
bool PowerPolicy::update(uint32_t now, bool activity, PowerSource source) {
    if (activity && state_ != IdleState::Awake) wake(now);
    if (activity) lastActivity_ = now;
    if (swallowing_) {
        if (activity) quietSince_ = now;
        if (!activity && now - quietSince_ >= 50) swallowing_ = false;
        return true;
    }
    if (state_ == IdleState::Sleeping) return true;
    const uint32_t idle = now - lastActivity_;
    if (!activity && idle >= SleepMs && source == PowerSource::Battery)
        state_ = IdleState::Sleeping;
    else if (!activity && idle >= DimMs) state_ = IdleState::Dim;
    return state_ != IdleState::Awake;
}
