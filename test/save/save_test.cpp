// Exercises real desktop persistence, validation, protected saves, and explicit reset.
#include "Save.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <string>

// Write a fixture exactly as a future/corrupt firmware could have left it.
static void fixture(const char* path, const SaveData& data) {
    FILE* file = fopen(path, "wb");
    assert(file);
    assert(fwrite(&data, sizeof(data), 1, file) == 1);
    assert(fclose(file) == 0);
}

int main() {
    char directory[MAX_PATH], path[MAX_PATH];
    assert(GetTempPathA(MAX_PATH, directory));
    assert(GetTempFileNameA(directory, "nz6", 0, path));
    assert(remove(path) == 0);
    assert(_putenv_s("NOURS_SAVE_PATH", path) == 0);
    Save save;
    assert(save.load() == SaveStatus::New);
    assert(save.data.volume == 1 && save.data.talks == 0);
    save.data.talks = 17;
    save.data.bondXp = 23;
    save.data.volume = 0;
    save.data.debug = 1;
    save.data.studyActive = 1;
    save.data.studyRemainingMs = 123456;
    save.data.seenCount = 2;
    strcpy(save.data.seen[0], "dodo.talk.jump");
    strcpy(save.data.seen[1], "dodo.greeting.hello");
    strcpy(save.data.lastId, "dodo.talk.jump");
    const bool written = save.write();
    if (!written) fprintf(stderr, "Save write failed: validation=%d status=%d Windows=%lu path=%s\n",
        static_cast<int>(validateSave(save.data)), static_cast<int>(save.status()), GetLastError(), path);
    assert(written);
    Save restored;
    assert(restored.load() == SaveStatus::Ready);
    assert(memcmp(&save.data, &restored.data, sizeof(SaveData)) == 0);

    SaveData future = save.data;
    future.version = SaveData::Version + 1;
    future.checksum = saveChecksum(future);
    fixture(path, future);
    assert(restored.load() == SaveStatus::Unsupported);
    assert(restored.data.talks == 0 && !restored.write());
    Save stillProtected;
    assert(stillProtected.load() == SaveStatus::Unsupported);
    assert(stillProtected.wipe());
    assert(restored.load() == SaveStatus::Ready && restored.data.talks == 0);

    SaveData invalid;
    defaultSave(invalid);
    invalid.talks = 99; // Checksum deliberately not updated.
    fixture(path, invalid);
    assert(restored.load() == SaveStatus::Corrupt && !restored.write());
    assert(restored.wipe());
    defaultSave(invalid);
    invalid.seenCount = 65;
    invalid.checksum = saveChecksum(invalid);
    assert(validateSave(invalid) == SaveStatus::Corrupt);
    defaultSave(invalid);
    invalid.seenCount = 2;
    strcpy(invalid.seen[0], "duplicate"); strcpy(invalid.seen[1], "duplicate");
    invalid.checksum = saveChecksum(invalid);
    assert(validateSave(invalid) == SaveStatus::Corrupt);
    defaultSave(invalid);
    memset(invalid.lastId, 'x', sizeof(invalid.lastId));
    invalid.checksum = saveChecksum(invalid);
    assert(validateSave(invalid) == SaveStatus::Corrupt);
    defaultSave(invalid);
    invalid.studyActive = 1;
    invalid.checksum = saveChecksum(invalid);
    assert(validateSave(invalid) == SaveStatus::Corrupt);

    // The existing fixture is a file, so creating a child path must fail.
    const std::string impossible = std::string(path) + "\\child.bin";
    assert(_putenv_s("NOURS_SAVE_PATH", impossible.c_str()) == 0);
    assert(!restored.write() && restored.status() == SaveStatus::IoError);
    assert(!restored.wipe());
    assert(remove(path) == 0);
    puts("Save tests passed: roundtrip, versions, corruption, bounds, reset, I/O failure.");
}
