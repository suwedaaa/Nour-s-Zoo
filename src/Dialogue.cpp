// Wraps complete words before revealing characters, preventing shifting text.
#include <string.h>
#include "Dialogue.h"

// Fixed storage avoids heap allocation during conversations.
void Dialogue::begin(Speaker speaker, const char* text, uint32_t now) {
    speaker_ = speaker;
    page_ = 0;
    visible_ = 0;
    lastCharacterMs_ = now;
    if (!wrap(text, speaker == Speaker::Dodo ? DodoColumns : PlayerColumns)) {
        // Never silently overflow or truncate a conversation.
        wrap("This line is too long.", speaker == Speaker::Dodo ? DodoColumns : PlayerColumns);
    }
}

// Collapse spaces, honour newlines, and split only words wider than a full row.
bool Dialogue::wrap(const char* text, int columns) {
    memset(rows_, 0, sizeof(rows_));
    rowCount_ = 1;
    int column = 0;
    if (text == nullptr) {
        return true;
    }
    while (*text != '\0') {
        if (*text == '\n') {
            if (rowCount_ == MaxRows) return false;
            ++rowCount_;
            column = 0;
            ++text;
            continue;
        }
        if (*text == ' ' || *text == '\t' || *text == '\r') {
            ++text;
            continue;
        }
        const char* end = text;
        while (*end && *end != ' ' && *end != '\n' && *end != '\t' && *end != '\r') ++end;
        const size_t wordLength = static_cast<size_t>(end - text);
        if (column && column + 1 + wordLength > static_cast<size_t>(columns)) {
            if (rowCount_ == MaxRows) return false;
            ++rowCount_;
            column = 0;
        }
        if (column) rows_[rowCount_ - 1][column++] = ' ';
        while (text != end) {
            if (column == columns) {
                if (rowCount_ == MaxRows) return false;
                ++rowCount_;
                column = 0;
            }
            rows_[rowCount_ - 1][column++] = *text++;
        }
    }
    return true;
}

// The typewriter catches up after slow frames and handles millis rollover.
size_t Dialogue::update(uint32_t now) {
    if (fullyVisible()) return 0;
    const uint32_t steps = (now - lastCharacterMs_) / CharacterMs;
    const size_t remaining = pageLength() - visible_;
    const size_t typed = steps < remaining ? steps : remaining;
    visible_ += typed;
    lastCharacterMs_ += steps * CharacterMs;
    return typed;
}

// Return true only when a tap confirms an already-complete final page.
bool Dialogue::confirm(uint32_t now) {
    if (!fullyVisible()) {
        visible_ = pageLength();
        return false;
    }
    if (hasNextPage()) {
        ++page_;
        visible_ = 0;
        lastCharacterMs_ = now;
        return false;
    }
    return true;
}

// Return an empty string for unused rows on the current page.
const char* Dialogue::row(int index) const {
    const int absolute = page_ * RowsPerPage + index;
    return index >= 0 && index < RowsPerPage && absolute < rowCount_ ? rows_[absolute] : "";
}

// Count only drawable characters; page and row boundaries cost no typing time.
size_t Dialogue::pageLength() const {
    size_t count = 0;
    for (int index = 0; index < RowsPerPage; ++index) count += strlen(row(index));
    return count;
}

// Reveal rows in order without changing their precomputed word wrapping.
size_t Dialogue::visibleInRow(int index) const {
    size_t remaining = visible_;
    for (int previous = 0; previous < index; ++previous) {
        const size_t length = strlen(row(previous));
        if (remaining < length) return 0;
        remaining -= length;
    }
    const size_t length = strlen(row(index));
    return remaining < length ? remaining : length;
}
