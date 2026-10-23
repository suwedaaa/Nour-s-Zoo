#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdarg>
uint32_t millis();
void simulatorLog(const char* text);
struct DesktopSerial {
    void begin(int) {}
    void println(const char* s) { simulatorLog(s); }
    void printf(const char* fmt, ...) {
        char text[512]; va_list args; va_start(args, fmt);
        vsnprintf(text, sizeof(text), fmt, args); va_end(args); simulatorLog(text);
    }
};
inline DesktopSerial Serial;
