// Renders PNG-derived RGB565 assets into one checked full-screen framebuffer.
#include <Arduino.h>
#ifndef NOURS_DESKTOP
#include <esp_heap_caps.h>
#endif
#include "Renderer.h"
#include "backgrounds/background1.h"
#include "animals/dodo/dodo1.h"

// Reserve the large buffer before peripherals can fragment the fallback heap.
bool Renderer::begin() {
    Serial.begin(115200);
#ifdef NOURS_DESKTOP
    frame_.setColorDepth(16);
    if (!frame_.createSprite(ScreenWidth, ScreenHeight)) return false;
    M5.begin();
    Serial.println("Desktop framebuffer: 320x240 RGB565");
    return true;
#else
    // Arduino initialises PSRAM before setup(). Sprite allocation does not
    // require the physical display to be initialised.
    const bool psramReady = psramFound();
    const size_t psramLargest = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    const size_t dmaLargest = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);
    Serial.printf("Framebuffer startup: PSRAM=%s, largest PSRAM=%u, largest DMA=%u bytes\n",
                  psramReady ? "detected" : "missing",
                  static_cast<unsigned>(psramLargest), static_cast<unsigned>(dmaLargest));
    frame_.setColorDepth(16);
    frame_.setPsram(true);
    // Installed M5GFX already falls back to internal DMA-capable RAM. Calling
    // createSprite again with setPsram(false) only repeats the same failed try.
    void* pixels = frame_.createSprite(ScreenWidth, ScreenHeight);
    M5.begin();
    M5.Display.setRotation(1);
    if (pixels == nullptr) {
        // Direct-draw an error once; never animate without a framebuffer.
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setTextSize(1);
        M5.Display.setCursor(12, 60);
        M5.Display.println("Framebuffer allocation failed");
        M5.Display.setCursor(12, 84);
        M5.Display.printf("PSRAM: %s", psramReady ? "detected" : "MISSING");
        M5.Display.setCursor(12, 104);
        M5.Display.printf("Largest PSRAM: %u bytes", static_cast<unsigned>(psramLargest));
        M5.Display.setCursor(12, 124);
        M5.Display.printf("Largest DMA: %u bytes", static_cast<unsigned>(dmaLargest));
        M5.Display.setCursor(12, 144);
        M5.Display.print("Need: 153604 contiguous bytes");
        M5.Display.setCursor(12, 180);
        M5.Display.print("Please photograph this screen.");
        Serial.println("Nour's Zoo: framebuffer allocation failed.");
        return false;
    }
    usingPsram_ = esp_ptr_external_ram(pixels);
    Serial.printf("Nour's Zoo: 320x240 RGB565, 153600 bytes, %s\n",
                  usingPsram_ ? "PSRAM" : "internal RAM");
    return true;
#endif
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
    drawDodo(left, top, AnimalScale);
}

// Share the sprite for the zoo and a larger 3x dialogue portrait for now.
void Renderer::drawDodo(int left, int top, int scale) {
    for (int y = 0; y < AnimalCanvasSize; ++y) {
        for (int x = 0; x < AnimalCanvasSize; ++x) {
            const uint16_t colour = animals_dodo_dodo1_pixels[y * AnimalCanvasSize + x];
            if (colour != animals_dodo_dodo1_transparent) {
                frame_.fillRect(left + x * scale, top + y * scale,
                                scale, scale, colour);
            }
        }
    }
}

// Compose the entire scene, then perform exactly one display push.
void Renderer::drawZoo(ZooScreen screen, const Dialogue& dialogue, int selectedAction, float fps,
                       const DialogueNode* choiceNode, int selectedChoice,
                       const StudyTimer& study, int selectedSetting, const char* volume) {
    drawScene();
    frame_.setFont(&fonts::Font0); // Fixed 6x8 bitmap cells, including punctuation.
    frame_.setTextWrap(false, false); // Dialogue supplies its own stable wrapping.
    frame_.setTextColor(TFT_WHITE, TFT_BLACK);
    if (screen == ZooScreen::Dialogue) drawDialogue(dialogue);
    if (screen == ZooScreen::Actions) drawActions(selectedAction);
    if (screen == ZooScreen::Choices && choiceNode) drawChoices(*choiceNode, selectedChoice);
    if (screen == ZooScreen::Study) drawStudy(study);
    if (screen == ZooScreen::SoundSettings) drawSoundSettings(selectedSetting, volume);
    // Keep performance diagnostics available without obscuring normal play.
    constexpr bool ShowDebug = false;
    if (ShowDebug) {
        frame_.setTextSize(1);
        frame_.setCursor(2, 2);
        frame_.printf("FPS %.1f | state %d", fps, static_cast<int>(screen));
    }
    frame_.pushSprite(0, 0);
}

// Dodo has a left portrait; player dialogue uses the full width without a portrait.
void Renderer::drawDialogue(const Dialogue& dialogue) {
    frame_.fillRect(2, 136, 316, 102, TFT_BLACK);
    frame_.drawRect(2, 136, 316, 102, TFT_WHITE);
    frame_.drawRect(3, 137, 314, 100, TFT_WHITE);
    const bool dodo = dialogue.speaker() == Speaker::Dodo;
    const bool narration = dialogue.speaker() == Speaker::Narrator;
    if (dodo) drawDodo(10, 157, 3);
    const int textLeft = dodo ? 106 : narration ? 18 : 30;
    frame_.setTextSize(1);
    frame_.setCursor(narration ? textLeft : textLeft - 12, 146);
    frame_.print(dodo ? "DODO" : dialogue.speaker() == Speaker::Player ? "NOUR" : "NARRATION");
    frame_.setTextSize(2);
    if (!narration) {
        frame_.setCursor(textLeft - 12, 166);
        frame_.print("*");
    }
    for (int row = 0; row < Dialogue::RowsPerPage; ++row) {
        frame_.setCursor(textLeft, 166 + row * 18);
        const char* text = dialogue.row(row);
        const size_t visible = dialogue.visibleInRow(row);
        for (size_t index = 0; index < visible; ++index) frame_.write(text[index]);
    }
}

// Action boxes deliberately have neither a speaker portrait nor an asterisk.
void Renderer::drawActions(int selectedAction) {
    frame_.fillRect(2, 174, 316, 64, TFT_BLACK);
    frame_.drawRect(2, 174, 316, 64, TFT_WHITE);
    frame_.drawRect(3, 175, 314, 62, TFT_WHITE);
    const char* labels[] = {"Talk", "Study", "Back"};
    const int itemLeft[] = {14, 112, 222};
    frame_.setTextSize(2);
    for (int index = 0; index < 3; ++index) {
        const int left = itemLeft[index];
        if (index == selectedAction) {
            frame_.fillTriangle(left, 188, left, 200, left + 8, 194, TFT_WHITE);
        }
        frame_.setCursor(left + 14, 187);
        frame_.print(labels[index]);
    }
}

// Replies require a choice: no Back item, portrait, asterisk or control hints.
void Renderer::drawChoices(const DialogueNode& node, int selectedChoice) {
    frame_.fillRect(2, 136, 316, 102, TFT_BLACK);
    frame_.drawRect(2, 136, 316, 102, TFT_WHITE);
    frame_.drawRect(3, 137, 314, 100, TFT_WHITE);
    frame_.setTextSize(2);
    for (int index = 0; index < node.choiceCount; ++index) {
        const int top = 148 + index * 28;
        if (index == selectedChoice) {
            frame_.fillTriangle(12, top + 2, 12, top + 14, 20, top + 8, TFT_WHITE);
        }
        frame_.setCursor(28, top);
        frame_.print(node.choices[index].label);
    }
}

// Render rounded-up seconds so the session starts at 25:00 and ends at 00:00.
void Renderer::drawStudy(const StudyTimer& study) {
    frame_.fillRect(2, 164, 316, 74, TFT_BLACK);
    frame_.drawRect(2, 164, 316, 74, TFT_WHITE);
    frame_.drawRect(3, 165, 314, 72, TFT_WHITE);
    frame_.setTextSize(1);
    frame_.setCursor(16, 174);
    frame_.print(study.completed() ? "STUDY COMPLETE" : "STUDY TIME");
    const uint32_t seconds = study.remainingSeconds();
    frame_.setTextSize(4);
    frame_.setCursor(100, 192);
    frame_.printf("%02lu:%02lu", static_cast<unsigned long>(seconds / 60),
                  static_cast<unsigned long>(seconds % 60));
}

// Long-B on the zoo opens these sound settings; volume zero mutes all sounds.
void Renderer::drawSoundSettings(int selectedSetting, const char* volume) {
    frame_.fillRect(2, 136, 316, 102, TFT_BLACK);
    frame_.drawRect(2, 136, 316, 102, TFT_WHITE);
    frame_.drawRect(3, 137, 314, 100, TFT_WHITE);
    frame_.setTextSize(1);
    frame_.setCursor(16, 146);
    frame_.print("SOUND");
    frame_.setTextSize(2);
    for (int index = 0; index < 2; ++index) {
        const int top = 166 + index * 34;
        if (index == selectedSetting) {
            frame_.fillTriangle(12, top + 2, 12, top + 14, 20, top + 8, TFT_WHITE);
        }
        frame_.setCursor(28, top);
        if (index == 0) frame_.printf("Volume: %s", volume);
        else frame_.print("Back");
    }
}
