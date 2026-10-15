// Native tests for dialogue pages, graph validity, branching and stable-ID history.
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "Dialogue.h"
#include "DialogueData.h"
#include "DialogueHistory.h"
#include "Conversation.h"

// Exercise every page and compare all non-whitespace characters to source text.
void checkLine(Speaker speaker, const char* text) {
    Dialogue dialogue;
    dialogue.begin(speaker, text, 0);
    const size_t limit = speaker == Speaker::Dodo ? Dialogue::DodoColumns : Dialogue::PlayerColumns;
    char actual[1024] = {};
    size_t length = 0;
    int pages = 0;
    do {
        for (int row = 0; row < Dialogue::RowsPerPage; ++row) {
            const char* line = dialogue.row(row);
            assert(strlen(line) <= limit);
            for (size_t i = 0; line[i]; ++i) {
                if (line[i] != ' ') actual[length++] = line[i];
            }
        }
        if (!dialogue.fullyVisible()) assert(!dialogue.confirm(1));
        assert(dialogue.fullyVisible());
        assert(++pages <= 8);
    } while (!dialogue.confirm(2));
    char expected[1024] = {};
    size_t expectedLength = 0;
    for (size_t i = 0; text[i]; ++i) {
        if (text[i] != ' ' && text[i] != '\n' && text[i] != '\t' && text[i] != '\r')
            expected[expectedLength++] = text[i];
    }
    assert(strcmp(actual, expected) == 0);
}

// Every branch must end; no unreachable content or invalid edges are permitted.
void checkPath(const DialogueScript& script, int index, bool* reached, int depth) {
    if (index == -1) return;
    assert(index >= 0 && index < script.nodeCount && depth < 32);
    reached[index] = true;
    const auto& node = script.nodes[index];
    if (node.choiceCount) {
        assert(node.choiceCount == 2 && node.choices && !node.text);
        for (int choice = 0; choice < node.choiceCount; ++choice) {
            assert(strlen(node.choices[choice].label) <= 23);
            checkPath(script, node.choices[choice].nextNode, reached, depth + 1);
        }
    } else {
        assert(node.text && !node.choices);
        checkLine(node.speaker, node.text);
        checkPath(script, node.nextNode, reached, depth + 1);
    }
}

// Stable strings, not table indices, define history across edits and reorderings.
void testHistory() {
    const DialogueNode node[] = {{Speaker::Dodo, "Hi!", -1, nullptr, 0}};
    DialogueScript scripts[] = {
        {"a", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, node, 1},
        {"b", DialogueCategory::Talk, DialogueTier::Normal, 0, 0, node, 1},
        {"locked", DialogueCategory::Talk, DialogueTier::Normal, 2, 3, node, 1},
        {"rare", DialogueCategory::Talk, DialogueTier::Rare, 0, 0, node, 1},
        {"hello", DialogueCategory::Greeting, DialogueTier::Normal, 0, 0, node, 1},
    };
    DialogueHistory history;
    auto pick = [&](uint16_t bond, uint32_t talks) {
        return history.pick(scripts, 5, DialogueCategory::Talk, DialogueTier::Normal, bond, talks, 0);
    };
    assert(strcmp(pick(0, 0)->id, "a") == 0);
    // Selecting or cancelling does not consume the entry.
    assert(strcmp(pick(0, 0)->id, "a") == 0);
    assert(history.markCompleted("a"));
    assert(strcmp(pick(0, 0)->id, "b") == 0);
    assert(history.markCompleted("b"));
    assert(history.markCompleted("rare"));
    assert(history.markCompleted("hello"));
    // Newly eligible dialogue is shown before resetting an existing cycle.
    assert(strcmp(pick(2, 3)->id, "locked") == 0);
    assert(history.markCompleted("locked"));
    assert(strcmp(pick(2, 3)->id, "a") == 0);
    assert(history.hasSeen("rare") && history.hasSeen("hello"));
    assert(history.markCompleted("a"));
    // Reorder the table and use a different string address for an existing ID.
    char copiedId[] = "a";
    scripts[0].id = copiedId;
    const DialogueScript temporary = scripts[0];
    scripts[0] = scripts[1];
    scripts[1] = temporary;
    assert(strcmp(pick(2, 3)->id, "b") == 0);
    // Locked entries must never stop a cycle of available entries.
    DialogueHistory lockedHistory;
    lockedHistory.markCompleted("a");
    lockedHistory.markCompleted("b");
    assert(lockedHistory.pick(scripts, 5, DialogueCategory::Talk, DialogueTier::Normal, 0, 0, 0));
    assert(!history.pick(scripts, 5, DialogueCategory::Talk, DialogueTier::Secret, 0, 0, 0));
    assert(history.pick(scripts, 5, DialogueCategory::Talk, DialogueTier::Rare, 0, 0, 0));
    assert(!history.hasEligible(scripts + 2, 1, DialogueCategory::Talk, DialogueTier::Normal, 2, 2));
    assert(!history.hasEligible(scripts + 2, 1, DialogueCategory::Talk, DialogueTier::Normal, 1, 3));
    assert(history.hasEligible(scripts + 2, 1, DialogueCategory::Talk, DialogueTier::Normal, 2, 3));
}

int main() {
    Dialogue dialogue;
    dialogue.begin(Speaker::Dodo, "Hello world!", 100);
    assert(dialogue.update(134) == 0);
    assert(dialogue.visibleInRow(0) == 0);
    assert(dialogue.update(135) == 1);
    assert(dialogue.visibleInRow(0) == 1);
    assert(!dialogue.confirm(136));
    assert(dialogue.update(136) == 0); // Instant reveal must not enqueue speech beeps.
    assert(dialogue.visibleInRow(0) == 12);
    assert(dialogue.confirm(137));
    dialogue.begin(Speaker::Dodo, "one two three four five", 0);
    assert(strcmp(dialogue.row(0), "one two three") == 0);
    assert(strcmp(dialogue.row(1), "four five") == 0);
    dialogue.begin(Speaker::Narrator, "one two three four five", 0);
    assert(strcmp(dialogue.row(0), "one two three four five") == 0);
    dialogue.begin(Speaker::Dodo, "first\nsecond\nthird\nfourth", 0);
    assert(dialogue.hasNextPage());
    assert(!dialogue.confirm(1));
    assert(!dialogue.confirm(2));
    assert(strcmp(dialogue.row(0), "fourth") == 0 && dialogue.visibleInRow(0) == 0);
    assert(!dialogue.confirm(3));
    assert(dialogue.confirm(4));
    checkLine(Speaker::Dodo, "abcdefghijklmnopqrstuvwxyz");
    checkLine(Speaker::Dodo, "  Hi!   NOUR?\nIt's me. ");
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

    assert(DialogueData::ScriptCount <= DialogueHistory::Capacity);
    int branchingScripts = 0;
    for (size_t i = 0; i < DialogueData::ScriptCount; ++i) {
        const auto& script = DialogueData::Scripts[i];
        assert(script.nodeCount <= 32);
        bool reached[32] = {};
        checkPath(script, 0, reached, 0);
        for (int index = 0; index < script.nodeCount; ++index) assert(reached[index]);
        for (size_t j = i + 1; j < DialogueData::ScriptCount; ++j)
            assert(strcmp(script.id, DialogueData::Scripts[j].id) != 0);

        Conversation conversation;
        assert(conversation.begin(&script));
        while (conversation.node() && !conversation.node()->choiceCount) assert(conversation.advance());
        if (conversation.finished()) continue;
        ++branchingScripts;
        assert(!conversation.advance());
        assert(!conversation.choose(-1) && !conversation.choose(2));
        Conversation first = conversation;
        Conversation second = conversation;
        assert(first.choose(0) && second.choose(1));
        assert(first.node()->speaker == Speaker::Player && second.node()->speaker == Speaker::Player);
        assert(strcmp(first.node()->text, second.node()->text) != 0);
        while (!first.finished()) assert(first.advance());
        while (!second.finished()) assert(second.advance());
        conversation.cancel();
        assert(!conversation.script() && !conversation.node());
    }
    assert(branchingScripts == 2);
    testHistory();
    puts("PASS: all dialogue pages and graph paths, both required replies, internal cancellation, IDs, eligibility and independent no-repeat cycles.");
    return 0;
}
