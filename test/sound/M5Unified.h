// Minimal speaker test double: lets native tests exercise the production Sound adapter.
#pragma once
#include <stdint.h>

struct TestSpeaker {
    bool available = true;
    uint8_t volume = 255;
    int tones = 0;
    int stops = 0;
    int channelStops = 0;
    float frequency = 0;
    uint32_t duration = 0;
    int channel = -1;
    bool replace = false;
    bool begin() { return available; }
    void setVolume(uint8_t value) { volume = value; }
    void stop() { ++stops; }
    void stop(uint8_t) { ++channelStops; }
    bool tone(float hz, uint32_t ms, int ch, bool interrupt) {
        ++tones; frequency = hz; duration = ms; channel = ch; replace = interrupt;
        return true;
    }
};
struct TestM5 { TestSpeaker Speaker; };
struct TestSerial { void println(const char*) {} };
extern TestM5 M5;
extern TestSerial Serial;
