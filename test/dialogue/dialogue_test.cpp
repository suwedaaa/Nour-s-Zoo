// Native tests for wrapping, typewriter timing, pagination and supplied dialogue.
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "Dialogue.h"
#include "DialogueData.h"

// Check that every page stays within the layout and can finish without truncation.
void checkLine(Speaker speaker, const char* text) {
    Dialogue dialogue;
    dialogue.begin(speaker, text, 0);
    const size_t limit = speaker == Speaker::Dodo ? Dialogue::DodoColumns : Dialogue::PlayerColumns;
    int pages = 0;
    do {
        for (int row = 0; row < Dialogue::RowsPerPage; ++row) {
            assert(strlen(dialogue.row(row)) <= limit);
            assert(strstr(dialogue.row(row), "too long") == nullptr);
        }
        if (!dialogue.fullyVisible()) assert(!dialogue.confirm(1));
        assert(dialogue.fullyVisible());
        ++pages;
        assert(pages <= 8);
    } while (!dialogue.confirm(2));
}

int main() {
    Dialogue dialogue;
    dialogue.begin(Speaker::Dodo, "Hello world!", 100);
    assert(dialogue.visibleInRow(0) == 0);
    dialogue.update(134);
    assert(dialogue.visibleInRow(0) == 0);
    dialogue.update(135);
    assert(dialogue.visibleInRow(0) == 1);
    // Reveal cannot also advance or close.
    assert(!dialogue.confirm(136));
    assert(dialogue.fullyVisible());
    assert(dialogue.visibleInRow(0) == 12);
    assert(dialogue.confirm(137));

    dialogue.begin(Speaker::Dodo, "one two three four five", 0);
    assert(strcmp(dialogue.row(0), "one two three") == 0);
    assert(strcmp(dialogue.row(1), "four five") == 0);
    assert(dialogue.visibleInRow(1) == 0);
    dialogue.update(14 * Dialogue::CharacterMs);
    assert(dialogue.visibleInRow(0) == 13);
    assert(dialogue.visibleInRow(1) == 1);

    dialogue.begin(Speaker::Player, "one two three four five", 0);
    assert(dialogue.speaker() == Speaker::Player);
    assert(strcmp(dialogue.row(0), "one two three four five") == 0);

    dialogue.begin(Speaker::Dodo, "first\nsecond\nthird\nfourth", 0);
    assert(dialogue.hasNextPage());
    assert(!dialogue.confirm(1)); // Reveal first page.
    assert(!dialogue.confirm(2)); // Advance, starting a fresh typewriter.
    assert(strcmp(dialogue.row(0), "fourth") == 0);
    assert(dialogue.visibleInRow(0) == 0);
    assert(!dialogue.hasNextPage());
    assert(!dialogue.confirm(3)); // Reveal second page.
    assert(dialogue.confirm(4));

    dialogue.begin(Speaker::Dodo, "abcdefghijklmnopqrstuvwxyz", 0);
    assert(strcmp(dialogue.row(0), "abcdefghijklmnopq") == 0);
    assert(strcmp(dialogue.row(1), "rstuvwxyz") == 0);
    dialogue.begin(Speaker::Dodo, "  Hi!   NOUR?\nIt's me. ", 0);
    assert(strcmp(dialogue.row(0), "Hi! NOUR?") == 0);
    assert(strcmp(dialogue.row(1), "It's me.") == 0);

    const uint32_t start = 0xFFFFFFF0u;
    dialogue.begin(Speaker::Dodo, "Hi", start);
    dialogue.update(start + 35u);
    assert(dialogue.visibleInRow(0) == 1);
    dialogue.update(start + 70u);
    assert(dialogue.fullyVisible());

    char oversized[1000];
    memset(oversized, 'x', sizeof(oversized) - 1);
    oversized[sizeof(oversized) - 1] = 0;
    dialogue.begin(Speaker::Player, oversized, 0);
    assert(strcmp(dialogue.row(0), "This line is too long.") == 0);
    dialogue.begin(Speaker::Player, nullptr, 0);
    assert(dialogue.confirm(0));

    for (int i = 0; i < DialogueData::GreetingCount; ++i) {
        checkLine(Speaker::Dodo, DialogueData::Greetings[i].text);
        for (int j = i + 1; j < DialogueData::GreetingCount; ++j) {
            assert(strcmp(DialogueData::Greetings[i].id, DialogueData::Greetings[j].id) != 0);
        }
    }
    checkLine(Speaker::Dodo, DialogueData::Talk.text);
    checkLine(Speaker::Player, DialogueData::StudyPreview.text);
    puts("PASS: wrapping, punctuation, typewriter, reveal/advance separation, pagination, overflow, rollover, supplied lines.");
    return 0;
}
