// Owns one atomic save blob and protects unsupported/corrupt saves from replacement.
#pragma once
#include "SaveData.h"

class Save {
public:
    SaveData data = {};
    SaveStatus load();
    bool write();
    bool wipe();
    SaveStatus status() const { return status_; }
    const char* label() const;
private:
    SaveStatus status_ = SaveStatus::New;
    bool protected_ = false;
    bool store();
};
