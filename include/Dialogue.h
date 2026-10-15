// Owns wrapped dialogue pages and typewriter timing, without input or drawing.
#pragma once
#include <stddef.h>
#include <stdint.h>

enum class Speaker : uint8_t { Dodo, Player };

class Dialogue {
public:
    static constexpr uint32_t CharacterMs = 35;
    static constexpr int RowsPerPage = 3;
    static constexpr int DodoColumns = 17;
    static constexpr int PlayerColumns = 23;
    static constexpr int MaxRows = 24;

    void begin(Speaker speaker, const char* text, uint32_t now);
    void update(uint32_t now);
    bool confirm(uint32_t now);
    Speaker speaker() const { return speaker_; }
    const char* row(int index) const;
    size_t visibleInRow(int index) const;
    bool fullyVisible() const { return visible_ >= pageLength(); }
    bool hasNextPage() const { return (page_ + 1) * RowsPerPage < rowCount_; }

private:
    char rows_[MaxRows][PlayerColumns + 1] = {};
    int rowCount_ = 1;
    int page_ = 0;
    size_t visible_ = 0;
    uint32_t lastCharacterMs_ = 0;
    Speaker speaker_ = Speaker::Dodo;

    bool wrap(const char* text, int columns);
    size_t pageLength() const;
};
