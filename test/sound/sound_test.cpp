// Tests actual Sound.cpp against a fake speaker; does not claim hardware audio verification.
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "M5Unified.h"
#include "Sound.h"

TestM5 M5;
TestSerial Serial;

int main() {
    Sound sound;
    sound.begin();
    assert(M5.Speaker.volume == 40 && strcmp(sound.volumeLabel(), "Low") == 0);
    sound.click();
    assert(M5.Speaker.tones == 1 && M5.Speaker.duration == 8);
    assert(M5.Speaker.channel == 0 && M5.Speaker.replace);
    sound.newSpeech();
    sound.typedCharacters(1);
    assert(M5.Speaker.tones == 1);
    sound.typedCharacters(1);
    assert(M5.Speaker.tones == 2 && M5.Speaker.frequency == 740);
    assert(M5.Speaker.channel == 1 && M5.Speaker.duration == 16);
    sound.typedCharacters(20);
    assert(M5.Speaker.tones == 3); // No burst after a slow frame.
    sound.typedCharacters(0);
    assert(M5.Speaker.tones == 3);
    sound.studyComplete();
    assert(M5.Speaker.duration == 180);
    sound.cycleVolume(); // Medium.
    assert(M5.Speaker.volume == 90);
    sound.cycleVolume(); // High.
    assert(M5.Speaker.volume == 150);
    sound.cycleVolume(); // Mute.
    assert(M5.Speaker.volume == 0 && M5.Speaker.stops == 1);
    const int count = M5.Speaker.tones;
    sound.click();
    sound.typedCharacters(2);
    sound.studyComplete();
    assert(M5.Speaker.tones == count);
    sound.cycleVolume(); // Low.
    assert(M5.Speaker.volume == 40 && M5.Speaker.tones == count + 1);
    M5.Speaker.available = false;
    Sound unavailable;
    unavailable.begin();
    const int before = M5.Speaker.tones;
    unavailable.click();
    unavailable.typedCharacters(2);
    unavailable.studyComplete();
    assert(M5.Speaker.tones == before);
    puts("PASS: clicks, Dodo pitch/cadence, coalesced beeps, completion note, all volume levels, mute and speaker failure.");
}
