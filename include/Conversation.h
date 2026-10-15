// Walks a data-only conversation graph independently of input, drawing and saves.
#pragma once
#include "DialogueData.h"

class Conversation {
public:
    bool begin(const DialogueScript* script);
    const DialogueNode* node() const;
    const DialogueScript* script() const { return script_; }
    bool advance();
    bool choose(int choice);
    bool finished() const { return nodeIndex_ == -1; }
    void cancel();

private:
    const DialogueScript* script_ = nullptr;
    int nodeIndex_ = -1;
    bool moveTo(int next);
};
