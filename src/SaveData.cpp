// Validates version, checksum and bounded fields before saved data affects gameplay.
#include <string.h>
#include "SaveData.h"
#include "StudyTimer.h"

void defaultSave(SaveData& data) {
    memset(&data, 0, sizeof(data));
    data.magic = SaveData::Magic;
    data.version = SaveData::Version;
    data.volume = 1;
    data.checksum = saveChecksum(data);
}

// FNV-1a detects accidental blob corruption, not malicious tampering.
uint32_t saveChecksum(const SaveData& data) {
    const auto* bytes = reinterpret_cast<const uint8_t*>(&data);
    uint32_t value = 2166136261u;
    for (size_t i = 0; i < offsetof(SaveData, checksum); ++i)
        value = (value ^ bytes[i]) * 16777619u;
    return value;
}

SaveStatus validateSave(const SaveData& data) {
    if (data.magic != SaveData::Magic) return SaveStatus::Corrupt;
    if (data.version != SaveData::Version) return SaveStatus::Unsupported;
    if (data.checksum != saveChecksum(data) || data.volume > 3 || data.debug > 1 ||
        data.studyActive > 1 || data.seenCount > SaveData::Capacity ||
        data.studyRemainingMs > StudyTimer::DefaultDurationMs ||
        (data.studyActive && !data.studyRemainingMs) ||
        !memchr(data.lastId, 0, SaveData::IdSize)) return SaveStatus::Corrupt;
    for (size_t i = 0; i < data.seenCount; ++i) {
        if (!data.seen[i][0] || !memchr(data.seen[i], 0, SaveData::IdSize)) return SaveStatus::Corrupt;
        for (size_t j = 0; j < i; ++j)
            if (strcmp(data.seen[i], data.seen[j]) == 0) return SaveStatus::Corrupt;
    }
    return SaveStatus::Ready;
}
