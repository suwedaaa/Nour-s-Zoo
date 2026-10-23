#include <windows.h>
#include <mmsystem.h>
#include <chrono>
#include <deque>
#include <string>
#include <cmath>
#include <fstream>
#include "M5Unified.h"
#include "Signals.h"
#define PROGMEM
#include "font5x7.h"

void setup();
void loop();
namespace {
HWND window=nullptr;
std::vector<uint32_t> screen(320*240);
std::deque<std::string> events;
uint8_t volume=90;
bool audioEnabled=true, headless=false;
COLORREF leds[10]{};
uint32_t vibrationUntil=0;
bool keyboard[3]{}, mouse[3]{};
struct Voice {
    HWAVEOUT device=nullptr;
    WAVEHDR header{};
    std::vector<short> samples;
    float frequency=0;
    uint32_t until=0;
    bool prepared=false;
} voices[2];
const auto start=std::chrono::steady_clock::now();
void stopAudio(Voice& voice) {
    if(voice.device && voice.prepared) {
        waveOutReset(voice.device);
        waveOutUnprepareHeader(voice.device,&voice.header,sizeof(WAVEHDR));
        voice.prepared=false;
    }
}
void label(HDC dc,int x,int y,const std::string& text) {
    TextOutA(dc,x,y,text.c_str(),static_cast<int>(text.size()));
}
void box(HDC dc,RECT rect,COLORREF color) {
    HBRUSH brush=CreateSolidBrush(color); FillRect(dc,&rect,brush); DeleteObject(brush);
}
void syncButtons() {
    DesktopButton* buttons[]={&M5.BtnA,&M5.BtnB,&M5.BtnC};
    for(int i=0;i<3;++i) {
        bool pressed=keyboard[i]||mouse[i];
        if(buttons[i]->down!=pressed) {
            buttons[i]->down=pressed;
            std::string message="Button "; message+=char('A'+i); message+=pressed?" down":" up";
            simulatorLog(message.c_str());
        }
    }
}
int keyIndex(WPARAM key) { return key=='A'?0:key=='S'?1:key=='D'?2:-1; }
void releaseAll() {
    for(int i=0;i<3;++i) keyboard[i]=mouse[i]=false;
    syncButtons();
}
void paint(HDC dc) {
    RECT all{0,0,1100,740}; box(dc,all,RGB(23,29,36));
    SetBkMode(dc,TRANSPARENT); SetTextColor(dc,RGB(230,237,240));
    SelectObject(dc,GetStockObject(DEFAULT_GUI_FONT));
    label(dc,30,18,"NOUR'S ZOO  /  Desktop Fire simulator");
    BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=320; info.bmiHeader.biHeight=-240;
    info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
    SetStretchBltMode(dc,COLORONCOLOR);
    StretchDIBits(dc,30,50,640,480,0,0,320,240,screen.data(),&info,DIB_RGB_COLORS,SRCCOPY);
    for(int i=0;i<3;++i) {
        RECT r{60+i*205,550,230+i*205,595};
        box(dc,r,(keyboard[i]||mouse[i])?RGB(42,153,145):RGB(61,73,86));
        std::string text="Button "; text+=char('A'+i); text+="  ["; text+="ASD"[i]; text+="]";
        label(dc,r.left+30,565,text);
    }
    label(dc,30,620,"Click and hold, or use A / S / D. Short S: interact. Hold S on zoo: sound settings.");
    label(dc,30,645,"M: PC audio on/off   |   L: LED test   |   V: vibration signal test   |   T: tone test");
    label(dc,30,670,"LED and vibration tests are simulator diagnostics; the game does not drive them yet.");
    label(dc,710,50,"OUTPUT SIGNALS");
    label(dc,710,78,audioEnabled?"PC audio: enabled (M to mute)":"PC audio: muted; signals still recorded");
    label(dc,710,104,"Game volume: "+std::to_string(volume)+" / 255");
    for(int i=0;i<2;++i) {
        char text[120]; bool active=millis()<voices[i].until;
        snprintf(text,sizeof(text),"Speaker %d: %s  | last %.0f Hz",i,active?"ACTIVE":"idle",voices[i].frequency);
        label(dc,710,140+i*26,text);
    }
    label(dc,710,206,"LED strip (10 RGB outputs)");
    for(int i=0;i<10;++i) box(dc,RECT{710+i*33,235,735+i*33,260},leds[i]?leds[i]:RGB(45,50,60));
    label(dc,710,282,millis()<vibrationUntil?"Vibration signal: ACTIVE":"Vibration signal: idle");
    label(dc,710,325,"EVENT LOG (newest first)");
    int y=355;
    for(const auto& event:events) { label(dc,710,y,event); y+=21; if(y>710) break; }
}
LRESULT CALLBACK procedure(HWND hwnd,UINT msg,WPARAM w,LPARAM l) {
    switch(msg) {
    case WM_KEYDOWN: {
        int i=keyIndex(w);
        if(i>=0) { keyboard[i]=true; syncButtons(); }
        else if(!(l & (1L<<30))) {
            if(w=='M') { audioEnabled=!audioEnabled; for(auto& voice:voices) stopAudio(voice); }
            if(w=='T') { simulatorLog("TEST: speaker tone"); simulatorTone(740,250,0); }
            if(w=='L') { static bool on=false; on=!on; simulatorLog("TEST: LED strip"); for(int n=0;n<10;++n) simulatorLed(n,0,on?160:0,on?255:0); }
            if(w=='V') { simulatorLog("TEST: vibration signal"); simulatorVibration(500); }
        }
        return 0;
    }
    case WM_KEYUP: { int i=keyIndex(w); if(i>=0) { keyboard[i]=false; syncButtons(); } return 0; }
    case WM_LBUTTONDOWN: {
        int x=LOWORD(l),y=HIWORD(l);
        for(int i=0;i<3;++i) if(x>=60+i*205 && x<230+i*205 && y>=550 && y<595) { mouse[i]=true; SetCapture(hwnd); }
        syncButtons(); return 0;
    }
    case WM_LBUTTONUP:
        for(bool& b:mouse) b=false;
        syncButtons(); ReleaseCapture(); return 0;
    case WM_CAPTURECHANGED:
        for(bool& b:mouse) b=false;
        syncButtons(); return 0;
    case WM_KILLFOCUS: releaseAll(); return 0;
    case WM_PAINT: { PAINTSTRUCT p; HDC dc=BeginPaint(hwnd,&p); paint(dc); EndPaint(hwnd,&p); return 0; }
    case WM_ERASEBKGND: return 1;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd,msg,w,l);
}
}
uint32_t millis() { return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count()); }
void simulatorLog(const char* text) {
    std::string clean(text); for(char& c:clean) if(c=='\n'||c=='\r') c=' ';
    events.push_front(std::to_string(millis())+"ms  "+clean);
    if(events.size()>100) events.pop_back();
    std::printf("%s\n",events.front().c_str());
}
void simulatorPresent(const std::vector<uint16_t>& pixels) {
    for(size_t i=0;i<std::min(pixels.size(),screen.size());++i) {
        uint16_t c=pixels[i]; screen[i]=((c>>11)*255/31<<16)|(((c>>5)&63)*255/63<<8)|((c&31)*255/31);
    }
    if(window) InvalidateRect(window,nullptr,FALSE);
}
void simulatorVolume(uint8_t v) { volume=v; }
void simulatorStop(int channel) {
    for(int i=0;i<2;++i) if(channel<0||channel==i) { stopAudio(voices[i]); voices[i].until=0; }
}
void simulatorTone(float frequency,uint32_t duration,int channel) {
    if(channel<0||channel>=2||frequency<=0) return;
    Voice& voice=voices[channel]; stopAudio(voice);
    voice.frequency=frequency; voice.until=millis()+duration;
    char event[120]; snprintf(event,sizeof(event),"Tone ch%d: %.0f Hz / %ums / vol %u",channel,frequency,duration,volume); simulatorLog(event);
    if(headless||!audioEnabled||!volume) return;
    if(!voice.device) {
        WAVEFORMATEX format{}; format.wFormatTag=WAVE_FORMAT_PCM; format.nChannels=1;
        format.nSamplesPerSec=44100; format.wBitsPerSample=16; format.nBlockAlign=2; format.nAvgBytesPerSec=88200;
        if(waveOutOpen(&voice.device,WAVE_MAPPER,&format,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR) {
            voice.device=nullptr; simulatorLog("Audio device unavailable; signals still work"); return;
        }
    }
    voice.samples.resize(static_cast<size_t>(44100ull*std::min(duration,10000u)/1000));
    for(size_t i=0;i<voice.samples.size();++i) {
        double envelope=std::min(1.0,std::min(i/88.0,(voice.samples.size()-i)/88.0));
        voice.samples[i]=static_cast<short>(std::sin(i*6.28318530718*frequency/44100)*volume/255.0*12000*envelope);
    }
    voice.header={}; voice.header.lpData=reinterpret_cast<char*>(voice.samples.data());
    voice.header.dwBufferLength=static_cast<DWORD>(voice.samples.size()*sizeof(short));
    if(waveOutPrepareHeader(voice.device,&voice.header,sizeof(WAVEHDR))==MMSYSERR_NOERROR) {
        voice.prepared=true;
        if(waveOutWrite(voice.device,&voice.header,sizeof(WAVEHDR))!=MMSYSERR_NOERROR) simulatorLog("Audio playback failed");
    }
}
void simulatorLed(int index,uint8_t red,uint8_t green,uint8_t blue) {
    if(index<0||index>=10) return;
    leds[index]=RGB(red,green,blue);
    char text[80]; snprintf(text,sizeof(text),"LED %d: RGB %u,%u,%u",index,red,green,blue); simulatorLog(text);
}
void simulatorVibration(uint32_t duration) {
    vibrationUntil=millis()+duration;
    simulatorLog(("Vibration signal: "+std::to_string(duration)+"ms").c_str());
}
void M5Canvas::write(char ch) {
    if(ch=='\n') { cursorX=0; cursorY+=8*scale; return; }
    unsigned char c=static_cast<unsigned char>(ch);
    fillRect(cursorX,cursorY,6*scale,8*scale,background);
    for(int x=0;x<5;++x) for(int y=0;y<8;++y)
        if(font[c*5+x] & (1<<y)) fillRect(cursorX+x*scale,cursorY+y*scale,scale,scale,foreground);
    cursorX+=6*scale;
}
void M5Canvas::fillTriangle(int ax,int ay,int bx,int by,int cx,int cy,uint16_t color) {
    auto edge=[](int x,int y,int a,int b,int c,int d) { return (x-a)*(d-b)-(y-b)*(c-a); };
    for(int y=std::min({ay,by,cy});y<=std::max({ay,by,cy});++y)
        for(int x=std::min({ax,bx,cx});x<=std::max({ax,bx,cx});++x) {
            int a=edge(x,y,ax,ay,bx,by), b=edge(x,y,bx,by,cx,cy), c=edge(x,y,cx,cy,ax,ay);
            if((a>=0&&b>=0&&c>=0)||(a<=0&&b<=0&&c<=0)) fillRect(x,y,1,1,color);
        }
}
int main(int argc,char** argv) {
    headless=argc>1 && std::string(argv[1])=="--self-test";
    if(headless) {
        setup();
        auto advance=[](int ms) { auto end=millis()+ms; while(millis()<end) { loop(); Sleep(1); } };
        advance(60); auto zoo=screen;
        keyboard[1]=true; syncButtons(); advance(100);
        keyboard[1]=false; syncButtons(); advance(250);
        if(zoo==screen || std::all_of(zoo.begin(),zoo.end(),[](uint32_t c){return c==0;})) return 1;
        keyboard[0]=true; syncButtons(); releaseAll(); if(M5.BtnA.isPressed()) return 2;
        simulatorLed(9,20,30,40); if(leds[9]!=RGB(20,30,40)) return 3;
        simulatorVibration(500); if(vibrationUntil<=millis()) return 4;
        simulatorTone(440,100,0); if(voices[0].frequency!=440) return 5;
        simulatorStop(0); if(voices[0].until!=0) return 6;
        // Render the actual window paint path to a bitmap for visual inspection.
        HDC dc=CreateCompatibleDC(nullptr);
        BITMAPINFO info{}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=1100; info.bmiHeader.biHeight=-740;
        info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32;
        void* bits=nullptr;
        HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,nullptr,0);
        if(!bitmap) return 7;
        HGDIOBJ previous=SelectObject(dc,bitmap); paint(dc); GdiFlush();
        BITMAPFILEHEADER header{}; header.bfType=0x4d42;
        header.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER); header.bfSize=header.bfOffBits+1100*740*4;
        std::ofstream capture("desktop-preview.bmp",std::ios::binary);
        capture.write(reinterpret_cast<char*>(&header),sizeof(header));
        capture.write(reinterpret_cast<char*>(&info.bmiHeader),sizeof(BITMAPINFOHEADER));
        capture.write(static_cast<char*>(bits),1100*740*4);
        SelectObject(dc,previous); DeleteObject(bitmap); DeleteDC(dc);
        std::puts("Desktop smoke test passed: rendered scene, dialogue input, release, LED, vibration, tone/stop.");
        return 0;
    }
    SetProcessDPIAware();
    WNDCLASSA wc{}; wc.lpfnWndProc=procedure; wc.hInstance=GetModuleHandle(nullptr);
    wc.lpszClassName="NoursZooSimulator"; wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
    RegisterClassA(&wc);
    RECT size{0,0,1100,740}; DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
    AdjustWindowRect(&size,style,FALSE);
    window=CreateWindowA(wc.lpszClassName,"Nour's Zoo - Desktop Simulator",style,CW_USEDEFAULT,CW_USEDEFAULT,size.right-size.left,size.bottom-size.top,nullptr,nullptr,wc.hInstance,nullptr);
    if(!window) return 1;
    ShowWindow(window,SW_SHOW); setup();
    MSG msg{}; bool running=true;
    while(running) {
        while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)) {
            if(msg.message==WM_QUIT) { running=false; break; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        if(running) { loop(); Sleep(1); }
    }
    simulatorStop(-1); for(auto& voice:voices) if(voice.device) waveOutClose(voice.device);
    return 0;
}
