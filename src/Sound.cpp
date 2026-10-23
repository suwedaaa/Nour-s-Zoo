// Uses M5Unified's asynchronous speaker task; never waits for a tone to finish.
#include <M5Unified.h>
#include "Sound.h"

namespace {
constexpr uint8_t Volumes[] = {0, 40, 90, 150};
const char* const Labels[] = {"Mute", "Low", "Medium", "High"};
}

// Called after M5.begin() and the early framebuffer allocation.
void Sound::begin() {
    applyVolume();
    ready_ = M5.Speaker.begin();
    if (!ready_) Serial.println("Speaker unavailable; continuing silently.");
}

// Every game sound honours the same master volume and mute setting.
void Sound::applyVolume() {
    M5.Speaker.setVolume(Volumes[volumeIndex_]);
    if (volumeIndex_ == 0) M5.Speaker.stop();
}

// Replace a channel's current tone instead of waiting or building a sound queue.
void Sound::tone(float frequency, uint32_t durationMs, int channel) {
    if (ready_ && volumeIndex_ != 0) {
        M5.Speaker.tone(frequency, durationMs, channel, true);
    }
}

// A short tick plays on the debounced down edge, including long-held presses.
void Sound::click() {
    tone(1400, 8, 0);
}

// Restart the cadence for each line and discard any residual speaking tone.
void Sound::newSpeech() {
    charactersUntilBeep_ = 0;
    if (ready_) M5.Speaker.stop(1);
}

// One Dodo-pitched blip per two naturally typed characters; slow frames coalesce
// into one blip. Reveals never call this method with the skipped characters.
void Sound::typedCharacters(size_t count) {
    charactersUntilBeep_ += count;
    if (charactersUntilBeep_ >= 2) {
        charactersUntilBeep_ %= 2;
        tone(740, 16, 1);
    }
}

// A single completion note, with no alarm loop.
void Sound::studyComplete() {
    tone(1046, 180, 0);
}

// Preview the new setting; Game persists it after this call.
void Sound::cycleVolume() {
    volumeIndex_ = (volumeIndex_ + 1) % 4;
    applyVolume();
    click();
}

// Expose a UI label without exposing the hardware speaker API.
const char* Sound::volumeLabel() const {
    return Labels[volumeIndex_];
}

// Restore a validated persisted setting without emitting a preview tone.
void Sound::setVolumeIndex(uint8_t value) {
    volumeIndex_ = value < 4 ? value : 1;
    applyVolume();
}

// Silence all channels before sleep while retaining the user's volume setting.
void Sound::silence() {
    if (ready_) M5.Speaker.stop();
}
