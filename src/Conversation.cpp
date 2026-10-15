// Advances only valid graph edges; choosing and advancing are separate operations.
#include "Conversation.h"

// Start at the first node; invalid data is rejected safely.
bool Conversation::begin(const DialogueScript* script) {
    cancel();
    if (!script || !script->nodes || script->nodeCount <= 0) return false;
    script_ = script;
    nodeIndex_ = 0;
    return true;
}

// Return no node after completion or cancellation.
const DialogueNode* Conversation::node() const {
    return script_ && nodeIndex_ >= 0 && nodeIndex_ < script_->nodeCount
        ? &script_->nodes[nodeIndex_] : nullptr;
}

// Reject an invalid edge without marking an unfinished conversation complete.
bool Conversation::moveTo(int next) {
    if (!script_ || next < -1 || next >= script_->nodeCount) return false;
    nodeIndex_ = next;
    return true;
}

// A choice cannot be skipped by a dialogue-advance button.
bool Conversation::advance() {
    const auto* current = node();
    return current && current->choiceCount == 0 && moveTo(current->nextNode);
}

// The selected reply leads to its own player line, narration and Dodo response.
bool Conversation::choose(int choice) {
    const auto* current = node();
    return current && current->choices && choice >= 0 && choice < current->choiceCount &&
           moveTo(current->choices[choice].nextNode);
}

// Cancelling does not mark history or award a completed talk.
void Conversation::cancel() {
    script_ = nullptr;
    nodeIndex_ = -1;
}
