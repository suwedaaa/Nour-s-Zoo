// Defines data-only conversation graphs, stable IDs, tiers and unlock requirements.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "Dialogue.h"

enum class DialogueTier : uint8_t { Normal, Rare, Secret };
enum class DialogueCategory : uint8_t { Greeting, Talk };

struct DialogueChoice {
    const char* label;
    int nextNode;
};

// A line has text and nextNode; a choice node has choices instead.
// nextNode == -1 finishes the conversation. Node positions are never saved.
struct DialogueNode {
    Speaker speaker;
    const char* text;
    int nextNode;
    const DialogueChoice* choices;
    int choiceCount;
};

struct DialogueScript {
    const char* id;
    DialogueCategory category;
    DialogueTier tier;
    uint16_t minimumBond;
    uint32_t minimumTalks;
    const DialogueNode* nodes;
    int nodeCount;
};

namespace DialogueData {
extern const DialogueScript Scripts[];
extern const size_t ScriptCount;
}
