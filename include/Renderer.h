// Isolates display hardware and owns the only full-screen RGB565 buffer.
#pragma once
#include <M5Unified.h>

class Renderer {
public:
    bool begin();
    void drawZoo(float fps);

private:
    static constexpr int ScreenWidth = 320;
    static constexpr int ScreenHeight = 240;
    static constexpr int AnimalCanvasSize = 24;
    static constexpr int AnimalScale = 2;
    M5Canvas frame_{&M5.Display};
    bool usingPsram_ = false;

    void drawScene();
};

