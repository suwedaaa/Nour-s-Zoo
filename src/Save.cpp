// Uses a single NVS blob on FIRE and an atomically replaced local file on desktop.
#include "Save.h"
#include <string.h>
#ifdef NOURS_DESKTOP
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
namespace {
// Test runs use an isolated override; normal simulator saves live in LocalAppData.
std::string savePath() {
    if (const char* overridePath = getenv("NOURS_SAVE_PATH")) return overridePath;
    const char* base = getenv("LOCALAPPDATA");
    std::string directory = std::string(base ? base : ".") + "\\NoursZoo";
    CreateDirectoryA(directory.c_str(), nullptr);
    return directory + "\\save.bin";
}
}
#else
#include <Preferences.h>
#endif

// Read into bounded storage; preserve unknown formats on disk for future migration.
SaveStatus Save::load() {
    defaultSave(data);
    protected_ = false;
#ifdef NOURS_DESKTOP
    FILE* file = fopen(savePath().c_str(), "rb");
    if (!file) {
        status_ = GetFileAttributesA(savePath().c_str()) == INVALID_FILE_ATTRIBUTES
            ? SaveStatus::New : SaveStatus::IoError;
        protected_ = status_ == SaveStatus::IoError;
        return status_;
    }
    fseek(file, 0, SEEK_END);
    const long length = ftell(file);
    rewind(file);
    const size_t read = fread(&data, 1, sizeof(data), file);
    fclose(file);
    status_ = length == sizeof(data) && read == sizeof(data)
        ? validateSave(data) : SaveStatus::Unsupported;
#else
    Preferences prefs;
    if (!prefs.begin("nourszoo", false)) {
        protected_ = true;
        return status_ = SaveStatus::IoError;
    }
    const size_t length = prefs.getBytesLength("state");
    if (!length) { prefs.end(); return status_ = SaveStatus::New; }
    const size_t read = prefs.getBytes("state", &data, sizeof(data));
    prefs.end();
    status_ = length == sizeof(data) && read == sizeof(data)
        ? validateSave(data) : SaveStatus::Unsupported;
#endif
    if (status_ != SaveStatus::Ready) {
        protected_ = true;
        defaultSave(data);
    }
    return status_;
}

// One committed NVS key avoids partially updated counters and history.
bool Save::store() {
#ifdef NOURS_DESKTOP
    const std::string path = savePath();
    const std::string temporary = path + ".tmp";
    FILE* file = fopen(temporary.c_str(), "wb");
    if (!file) return false;
    bool ok = fwrite(&data, 1, sizeof(data), file) == sizeof(data);
    if (fflush(file) != 0) ok = false;
    if (fclose(file) != 0) ok = false;
    return ok && MoveFileExA(temporary.c_str(), path.c_str(),
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
    Preferences prefs;
    if (!prefs.begin("nourszoo", false)) return false;
    const bool ok = prefs.putBytes("state", &data, sizeof(data)) == sizeof(data);
    prefs.end();
    return ok;
#endif
}

// Writes happen only when Game requests a meaningful-event checkpoint.
bool Save::write() {
    if (protected_) return false;
    data.checksum = saveChecksum(data);
    if (validateSave(data) != SaveStatus::Ready || !store()) {
        status_ = SaveStatus::IoError;
        return false;
    }
    status_ = SaveStatus::Ready;
    return true;
}

// Explicit reset replaces only this game's blob; no partition-wide erase.
bool Save::wipe() {
    defaultSave(data);
    data.checksum = saveChecksum(data);
    if (!store()) { status_ = SaveStatus::IoError; return false; }
    protected_ = false;
    status_ = SaveStatus::Ready;
    return true;
}

const char* Save::label() const {
    switch (status_) {
    case SaveStatus::Ready: return "Saved";
    case SaveStatus::New: return "New save";
    case SaveStatus::Unsupported: return "Version mismatch";
    case SaveStatus::Corrupt: return "Corrupt save";
    default: return "Save error";
    }
}
