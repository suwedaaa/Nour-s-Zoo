// Versioned, bounded save format; never serialises pointers or dialogue table indices.
#pragma once
#include <stdint.h>
#include <stddef.h>

struct SaveData {
    static constexpr uint32_t Magic = 0x4E5A4F4F;
    static constexpr uint32_t Version = 1;
    static constexpr size_t Capacity = 64;
    static constexpr size_t IdSize = 48;
    uint32_t magic;
    uint32_t version;
    uint32_t bondXp;
    uint32_t talks;
    uint32_t studyRemainingMs;
    uint8_t volume;
    uint8_t debug;
    uint8_t studyActive;
    uint8_t seenCount;
    char lastId[IdSize];
    char seen[Capacity][IdSize];
    uint32_t checksum;
};

enum class SaveStatus { Ready, New, Unsupported, Corrupt, IoError };

// These pure functions are shared by firmware and native tests.
void defaultSave(SaveData& data);
uint32_t saveChecksum(const SaveData& data);
SaveStatus validateSave(const SaveData& data);
