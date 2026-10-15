// Renders PNG-derived RGB565 assets into one checked full-screen framebuffer.
#include <Arduino.h>
#include <esp_heap_caps.h>
#include "Renderer.h"
#include "backgrounds/background1.h"
#include "animals/dodo/dodo1.h"

// Prefer PSRAM; retry in internal RAM only if the first allocation fails.
bool Renderer::begin() {
    M5.begin();
    Serial.begin(115200);
    M5.Display.setRotation(1);
    frame_.setColorDepth(16);
    frame_.setPsram(true);
    void* pixels = frame_.createSprite(ScreenWidth, ScreenHeight);
    if (pixels == nullptr) {
        frame_.setPsram(false);
        pixels = frame_.createSprite(ScreenWidth, ScreenHeight);
    }
    if (pixels == nullptr) {
        // Direct-draw an error once; never animate without a framebuffer.
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setTextSize(1);
        M5.Display.setCursor(12, 100);
        M5.Display.print("Framebuffer unavailable. Restart.");
        Serial.println("Nour's Zoo: framebuffer allocation failed.");
        return false;
    }
    usingPsram_ = esp_ptr_external_ram(pixels);
    Serial.printf("Nour's Zoo: 320x240 RGB565, 153600 bytes, %s\n",
                  usingPsram_ ? "PSRAM" : "internal RAM");
    return true;
}

// Draw an opaque background, then the keyed sprite at its feet anchor.
void Renderer::drawScene() {
    static_assert(backgrounds_background1_width == ScreenWidth &&
                  backgrounds_background1_height == ScreenHeight,
                  "Background must match the display");
    static_assert(animals_dodo_dodo1_width == AnimalCanvasSize &&
                  animals_dodo_dodo1_height == AnimalCanvasSize,
                  "Animals must use the shared canvas size");

    // Typed RGB565 pixels avoid ambiguity about packed colour byte order.
    frame_.pushImage(0, 0, ScreenWidth, ScreenHeight,
                     reinterpret_cast<const lgfx::rgb565_t*>(backgrounds_background1_pixels));

    const int left = 160 - animals_dodo_dodo1_anchor_x * AnimalScale;
    const int top = 150 - animals_dodo_dodo1_anchor_y * AnimalScale;
    // Integer-sized blocks provide nearest-neighbour 2x scaling without a
    // second sprite buffer. Transparent pixels leave the background intact.
    for (int y = 0; y < AnimalCanvasSize; ++y) {
        for (int x = 0; x < AnimalCanvasSize; ++x) {
            const uint16_t colour = animals_dodo_dodo1_pixels[y * AnimalCanvasSize + x];
            if (colour != animals_dodo_dodo1_transparent) {
                frame_.fillRect(left + x * AnimalScale, top + y * AnimalScale,
                                AnimalScale, AnimalScale, colour);
            }
        }
    }
}

// Compose the entire scene, then perform exactly one display push.
void Renderer::drawZoo(float fps, const InputTestView& inputTest) {
    drawScene();
    drawInputTest(fps, inputTest);
    frame_.pushSprite(0, 0);
}

// Persistent counters expose duplicate events, including a short after a long.
void Renderer::drawInputTest(float fps, const InputTestView& inputTest) {
    frame_.fillRect(0, 158, ScreenWidth, 82, TFT_BLACK);
    frame_.drawRect(0, 158, ScreenWidth, 82, TFT_WHITE);
    frame_.setTextSize(1);
    frame_.setTextColor(TFT_WHITE, TFT_BLACK);
    frame_.setCursor(6, 164);
    frame_.printf("INPUT TEST | 700ms hold | %.1f FPS", fps);
    for (int index = 0; index < 3; ++index) {
        frame_.setCursor(6, 178 + index * 12);
        frame_.printf("%c %-4s  SHORT:%lu  LONG:%lu", 'A' + index,
                      inputTest.down[index] ? "DOWN" : "UP",
                      static_cast<unsigned long>(inputTest.shortCount[index]),
                      static_cast<unsigned long>(inputTest.longCount[index]));
    }
    frame_.setCursor(6, 223);
    frame_.print(inputTest.lastEvent);
}
