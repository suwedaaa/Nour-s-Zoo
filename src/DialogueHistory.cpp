// Applies eligibility before no-repeat, so locked entries never block a cycle.
#include <string.h>
#include "DialogueHistory.h"

// Categories and tiers have independent cycles.
bool DialogueHistory::eligible(const DialogueScript& script, DialogueCategory category,
                               DialogueTier tier, uint16_t bond, uint32_t talks) {
    return script.category == category && script.tier == tier &&
           script.minimumBond <= bond && script.minimumTalks <= talks;
}

// Permanent IDs remain meaningful when data tables are reordered.
bool DialogueHistory::hasSeen(const char* id) const {
    for (size_t i = 0; i < seenCount_; ++i) {
        if (strcmp(seen_[i], id) == 0) return true;
    }
    return false;
}

// Called only after the entire selected branch reaches its end.
bool DialogueHistory::markCompleted(const char* id) {
    if (!id) return false;
    if (!hasSeen(id)) {
        if (seenCount_ == Capacity) return false;
        seen_[seenCount_++] = id;
    }
    lastCompleted_ = id;
    return true;
}

// Remove one ID without disturbing history for other pools or locked entries.
void DialogueHistory::forget(const char* id) {
    for (size_t i = 0; i < seenCount_; ++i) {
        if (strcmp(seen_[i], id) == 0) {
            seen_[i] = seen_[--seenCount_];
            return;
        }
    }
}

// Used to avoid rolling a rarity tier that has no unlocked conversations.
bool DialogueHistory::hasEligible(const DialogueScript* scripts, size_t count,
                                 DialogueCategory category, DialogueTier tier,
                                 uint16_t bond, uint32_t talks) const {
    for (size_t i = 0; i < count; ++i) {
        if (eligible(scripts[i], category, tier, bond, talks)) return true;
    }
    return false;
}

// Randomness is supplied by the caller, making selection deterministic in tests.
const DialogueScript* DialogueHistory::pick(const DialogueScript* scripts, size_t count,
                                           DialogueCategory category, DialogueTier tier,
                                           uint16_t bond, uint32_t talks, uint32_t random) {
    size_t available = 0;
    size_t unseen = 0;
    for (size_t i = 0; i < count; ++i) {
        if (!eligible(scripts[i], category, tier, bond, talks)) continue;
        ++available;
        if (!hasSeen(scripts[i].id)) ++unseen;
    }
    if (available == 0) return nullptr;
    const bool newCycle = unseen == 0;
    if (newCycle) {
        for (size_t i = 0; i < count; ++i) {
            if (eligible(scripts[i], category, tier, bond, talks)) forget(scripts[i].id);
        }
    }
    // Avoid repeating the immediately previous conversation across a cycle
    // boundary when another eligible entry exists.
    size_t candidates = 0;
    for (size_t i = 0; i < count; ++i) {
        const auto& script = scripts[i];
        if (!eligible(script, category, tier, bond, talks) || hasSeen(script.id)) continue;
        if (newCycle && available > 1 && lastCompleted_ && strcmp(script.id, lastCompleted_) == 0) continue;
        ++candidates;
    }
    if (!candidates) return nullptr;
    size_t chosen = random % candidates;
    for (size_t i = 0; i < count; ++i) {
        const auto& script = scripts[i];
        if (!eligible(script, category, tier, bond, talks) || hasSeen(script.id)) continue;
        if (newCycle && available > 1 && lastCompleted_ && strcmp(script.id, lastCompleted_) == 0) continue;
        if (chosen-- == 0) return &script;
    }
    return nullptr;
}
