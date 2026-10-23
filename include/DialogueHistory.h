// Tracks completed stable IDs and selects unseen eligible conversations per pool.
#pragma once
#include "DialogueData.h"

class DialogueHistory {
public:
    static constexpr size_t Capacity = 64;
    const DialogueScript* pick(const DialogueScript* scripts, size_t count,
                              DialogueCategory category, DialogueTier tier,
                              uint16_t bond, uint32_t talks, uint32_t random);
    bool markCompleted(const char* id);
    bool hasSeen(const char* id) const;
    size_t count() const { return seenCount_; }
    const char* idAt(size_t index) const { return index < seenCount_ ? seen_[index] : ""; }
    const char* lastId() const { return lastCompleted_ ? lastCompleted_ : ""; }
    void restoreLast(const char* id) { lastCompleted_ = id; }
    bool hasEligible(const DialogueScript* scripts, size_t count,
                     DialogueCategory category, DialogueTier tier,
                     uint16_t bond, uint32_t talks) const;

private:
    // IDs point into permanent tables; comparisons use contents, never addresses
    // or array positions. A later Save component can serialise these strings.
    const char* seen_[Capacity] = {};
    size_t seenCount_ = 0;
    const char* lastCompleted_ = nullptr;
    static bool eligible(const DialogueScript& script, DialogueCategory category,
                         DialogueTier tier, uint16_t bond, uint32_t talks);
    void forget(const char* id);
};
