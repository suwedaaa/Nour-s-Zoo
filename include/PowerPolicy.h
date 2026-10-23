// Pure inactivity policy: wake gestures are swallowed through their release.
#pragma once
#include <stdint.h>

enum class PowerSource { Battery, External, Unknown };
enum class IdleState { Awake, Dim, Sleeping };

class PowerPolicy {
public:
    static constexpr uint32_t DimMs = 60000;
    static constexpr uint32_t SleepMs = 300000;
    void begin(uint32_t now);
    bool update(uint32_t now, bool activity, PowerSource source);
    void wake(uint32_t now);
    void sleep(uint32_t now);
    IdleState state() const { return state_; }
private:
    uint32_t lastActivity_ = 0;
    uint32_t quietSince_ = 0;
    bool swallowing_ = false;
    IdleState state_ = IdleState::Awake;
};
