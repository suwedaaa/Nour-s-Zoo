// Isolates speaker hardware, volume and short non-blocking game sounds.
#pragma once
#include <stddef.h>
#include <stdint.h>

class Sound {
public:
    void begin();
    void click();
    void newSpeech();
    void typedCharacters(size_t count);
    void studyComplete();
    void cycleVolume();
    const char* volumeLabel() const;
    uint8_t volumeIndex() const { return volumeIndex_; }
    void setVolumeIndex(uint8_t value);
    void silence();

private:
    bool ready_ = false;
    uint8_t volumeIndex_ = 1;
    size_t charactersUntilBeep_ = 0;
    void applyVolume();
    void tone(float frequency, uint32_t durationMs, int channel);
};
