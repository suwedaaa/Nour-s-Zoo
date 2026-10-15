// Dodo's conversations: dialogue, narration and reply-dependent responses.
#include "DialogueData.h"

namespace {
const DialogueNode hello[] = {
    {Speaker::Narrator, "Dodo straightens up. This is apparently an official visit.", 1, nullptr, 0},
    {Speaker::Dodo, "NOUR! I have been standing here with incredible purpose.", -1, nullptr, 0},
};
const DialogueNode bread[] = {
    {Speaker::Dodo, "Do you have bread? No pressure. Only a very specific hope.", 1, nullptr, 0},
    {Speaker::Narrator, "He watches your hands with heroic optimism.", -1, nullptr, 0},
};
const DialogueNode flight[] = {
    {Speaker::Narrator, "Dodo gives his wings a tiny, determined shake.", 1, nullptr, 0},
    {Speaker::Dodo, "I tried flying today. The ground was extremely supportive.", -1, nullptr, 0},
};
const DialogueNode pond[] = {
    {Speaker::Dodo, "The pond called me round. I have decided it is a compliment.", 1, nullptr, 0},
    {Speaker::Narrator, "He admires his reflection. It admires him back.", -1, nullptr, 0},
};
const DialogueNode management[] = {
    {Speaker::Narrator, "Dodo stands on a pebble to address his employees. There are none.", 1, nullptr, 0},
    {Speaker::Dodo, "I have appointed myself head of snacks.", 2, nullptr, 0},
    {Speaker::Player, "A demanding job.", 3, nullptr, 0},
    {Speaker::Dodo, "My first decision? More snacks!", 4, nullptr, 0},
    {Speaker::Narrator, "He nods solemnly. The pebble has full confidence in him.", -1, nullptr, 0},
};
const DialogueNode leaf[] = {
    {Speaker::Narrator, "Dodo pushes a leaf towards you, then looks away shyly.", 1, nullptr, 0},
    {Speaker::Dodo, "I found this. It looked like something you would like.", 2, nullptr, 0},
    {Speaker::Player, "I'll keep it.", 3, nullptr, 0},
    {Speaker::Narrator, "His feathers puff up. He is trying very hard to look casual.", -1, nullptr, 0},
};
const DialogueNode cloud[] = {
    {Speaker::Dodo, "That cloud looks like me.", 1, nullptr, 0},
    {Speaker::Narrator, "The cloud is a long, thin streak. Dodo squints.", 2, nullptr, 0},
    {Speaker::Dodo, "Me, travelling very fast.", -1, nullptr, 0},
};
const DialogueChoice wingChoices[] = {
    {"You're doing great.", 3},
    {"Try a smaller jump?", 6},
};
const DialogueNode wings[] = {
    {Speaker::Narrator, "Dodo hops, lands, and stares at his feet. His wings droop.", 1, nullptr, 0},
    {Speaker::Dodo, "Was that flying? Please be kind to science.", 2, nullptr, 0},
    {Speaker::Player, nullptr, -1, wingChoices, 2},
    {Speaker::Player, "You're doing great.", 4, nullptr, 0},
    {Speaker::Narrator, "He lifts his head. One feather stands proudly to attention.", 5, nullptr, 0},
    {Speaker::Dodo, "Then I shall continue being great at ground level!", -1, nullptr, 0},
    {Speaker::Player, "Try a smaller jump?", 7, nullptr, 0},
    {Speaker::Narrator, "He performs a hop so small it may have been a thought.", 8, nullptr, 0},
    {Speaker::Dodo, "Perfect. A flight the ground could keep up with.", -1, nullptr, 0},
};
const DialogueChoice crownChoices[] = {
    {"Your Majesty.", 3},
    {"That's a leaf.", 6},
};
const DialogueNode crown[] = {
    {Speaker::Narrator, "A leaf rests on Dodo's head. He has become unbearable.", 1, nullptr, 0},
    {Speaker::Dodo, "I am accepting royal greetings.", 2, nullptr, 0},
    {Speaker::Player, nullptr, -1, crownChoices, 2},
    {Speaker::Player, "Your Majesty.", 4, nullptr, 0},
    {Speaker::Dodo, "You may have the finest pebble in my kingdom.", 5, nullptr, 0},
    {Speaker::Narrator, "He selects one with tremendous care. It is warm from the sun.", -1, nullptr, 0},
    {Speaker::Player, "That's a leaf.", 7, nullptr, 0},
    {Speaker::Narrator, "Dodo pauses. The leaf slides over one eye.", 8, nullptr, 0},
    {Speaker::Dodo, "A constitutional leaf.", -1, nullptr, 0},
};
const DialogueNode night[] = {
    {Speaker::Narrator, "For once, Dodo is quiet. He settles beside you.", 1, nullptr, 0},
    {Speaker::Dodo, "When you're here, I don't mind being a bird who stays.", 2, nullptr, 0},
    {Speaker::Narrator, "He leans against your shoe, then pretends he was inspecting it.", -1, nullptr, 0},
};
const DialogueNode secret[] = {
    {Speaker::Narrator, "Dodo checks behind a bush. Then behind a much smaller bush.", 1, nullptr, 0},
    {Speaker::Dodo, "A secret: sometimes I don't want bread.", 2, nullptr, 0},
    {Speaker::Narrator, "He waits for this information to settle.", 3, nullptr, 0},
    {Speaker::Dodo, "Sometimes I want two breads.", -1, nullptr, 0},
};
}

// Permanent IDs survive table reordering. Keep existing IDs when editing content.
namespace DialogueData {
const DialogueScript Scripts[] = {
    {"dodo.greeting.hello.001", DialogueCategory::Greeting, DialogueTier::Normal, 0, 0, hello, 2},
    {"dodo.greeting.bread.001", DialogueCategory::Greeting, DialogueTier::Normal, 0, 0, bread, 2},
    {"dodo.greeting.flight.001", DialogueCategory::Greeting, DialogueTier::Normal, 0, 0, flight, 2},
    {"dodo.greeting.pond.001", DialogueCategory::Greeting, DialogueTier::Normal, 0, 0, pond, 2},
    {"dodo.talk.management.001", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, management, 5},
    {"dodo.talk.leaf.001", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, leaf, 4},
    {"dodo.talk.cloud.001", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, cloud, 3},
    {"dodo.talk.wings.001", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, wings, 9},
    {"dodo.talk.crown.001", DialogueCategory::Talk, DialogueTier::Rare, 0, 3, crown, 9},
    {"dodo.talk.night.001", DialogueCategory::Talk, DialogueTier::Rare, 0, 3, night, 3},
    {"dodo.talk.secretbread.001", DialogueCategory::Talk, DialogueTier::Secret, 0, 6, secret, 4},
};
const size_t ScriptCount = sizeof(Scripts) / sizeof(Scripts[0]);
}
