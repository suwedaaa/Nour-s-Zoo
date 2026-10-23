#pragma once
#include "Arduino.h"
#include <vector>
#include <algorithm>
constexpr uint16_t TFT_BLACK = 0, TFT_WHITE = 0xffff;
namespace lgfx { using rgb565_t = uint16_t; }
namespace fonts { inline int Font0; }
void simulatorPresent(const std::vector<uint16_t>& pixels);
void simulatorTone(float frequency, uint32_t duration, int channel);
void simulatorStop(int channel);
void simulatorVolume(uint8_t volume);
struct DesktopButton {
    bool down = false;
    void setDebounceThresh(int) {}
    bool isPressed() const { return down; }
};
struct DesktopSpeaker {
    bool begin() { return true; }
    void setVolume(uint8_t v) { simulatorVolume(v); }
    void stop(int channel = -1) { simulatorStop(channel); }
    void tone(float hz, uint32_t ms, int channel, bool) { simulatorTone(hz, ms, channel); }
};
class M5Canvas {
public:
    explicit M5Canvas(void* = nullptr) {}
    void setColorDepth(int) {}
    void setPsram(bool) {}
    void* createSprite(int w, int h) { width=w; height=h; pixels.resize(w*h); return pixels.data(); }
    void setRotation(int) {}
    void setFont(const int*) {}
    void setTextWrap(bool, bool) {}
    void setTextSize(int s) { scale=s; }
    void setTextColor(uint16_t f, uint16_t b) { foreground=f; background=b; }
    void setCursor(int x, int y) { cursorX=x; cursorY=y; }
    void fillRect(int x, int y, int w, int h, uint16_t c) {
        for(int j=std::max(0,y); j<std::min(height,y+h); ++j)
            for(int i=std::max(0,x); i<std::min(width,x+w); ++i) pixels[j*width+i]=c;
    }
    void fillScreen(uint16_t c) { fillRect(0,0,width,height,c); }
    void drawRect(int x,int y,int w,int h,uint16_t c) {
        fillRect(x,y,w,1,c); fillRect(x,y+h-1,w,1,c); fillRect(x,y,1,h,c); fillRect(x+w-1,y,1,h,c);
    }
    void fillTriangle(int ax,int ay,int bx,int by,int cx,int cy,uint16_t color);
    void pushImage(int x,int y,int w,int h,const uint16_t* data) {
        for(int j=0;j<h;++j) for(int i=0;i<w;++i) fillRect(x+i,y+j,1,1,data[j*w+i]);
    }
    void write(char c);
    void print(const char* s) { while(*s) write(*s++); }
    void println(const char* s) { print(s); write('\n'); }
    void printf(const char* fmt, ...) {
        char text[512]; va_list args; va_start(args,fmt); vsnprintf(text,sizeof(text),fmt,args); va_end(args); print(text);
    }
    void pushSprite(int,int) { simulatorPresent(pixels); }
private:
    int width=0,height=0,cursorX=0,cursorY=0,scale=1;
    uint16_t foreground=TFT_WHITE,background=TFT_BLACK;
    std::vector<uint16_t> pixels;
};
struct DesktopM5 {
    M5Canvas Display;
    DesktopButton BtnA, BtnB, BtnC;
    DesktopSpeaker Speaker;
    void begin() {}
    void update() {}
};
inline DesktopM5 M5;
