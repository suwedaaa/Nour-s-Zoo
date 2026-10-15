// Dialogue content and permanent IDs; no drawing or button handling lives here.
#pragma once
#include <stdint.h>

enum class DialogueTier : uint8_t { Normal, Rare, Secret };

struct DialogueLine {
    const char* id;
    DialogueTier tier;
    uint16_t minimumBond;
    const char* text;
};

namespace DialogueData {
static const DialogueLine Greetings[] = {
    {"dodo.greeting.hello.001", DialogueTier::Normal, 0,
     "NOUR! I have been standing here with incredible purpose."},
    {"dodo.greeting.bread.001", DialogueTier::Normal, 0,
     "Do you have bread? No pressure. Only a very specific hope."},
    {"dodo.greeting.flight.001", DialogueTier::Normal, 0,
     "I tried flying today. The ground was extremely supportive."},
    {"dodo.greeting.pond.001", DialogueTier::Normal, 0,
     "The pond called me round. I have decided it is a compliment."},
};
static constexpr int GreetingCount = sizeof(Greetings) / sizeof(Greetings[0]);
static const DialogueLine Talk = {
    "dodo.talk.management.001", DialogueTier::Normal, 0,
    "I have appointed myself head of snacks. My first decision? More snacks!"
};
static const DialogueLine StudyPreview = {
    "player.study.preview.001", DialogueTier::Normal, 0,
    "A quiet spot to study. The study timer will be added in a later milestone."
};
}
