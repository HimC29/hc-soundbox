#pragma once
#include <Arduino.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <Bounce2.h>
#include <Adafruit_SSD1306.h>

// SD
#define SD_CS 5

// Audio
extern AudioFileSourceSD* source;
extern AudioGeneratorMP3* mp3Decoder;
extern AudioGeneratorWAV* wavDecoder;
extern AudioOutputI2S* output;

// Structs
struct DirContents {
    String* fileNames;
    int fileCount;
    bool* isDir;
};
extern DirContents dirContents;

struct MenuState {
    int selectedIndex;
    int scrollOffset;
};
extern MenuState menuState;

struct SongInfo {
    String format;
    String name;
    String fileLocation;
    unsigned long length;
    unsigned long startTime;
    bool paused;
    uint32_t savedPos;
    unsigned long pausedAt;
};
extern SongInfo songInfo;

struct ScrollState {
    int offset;
    unsigned long lastScrollTime;
};
extern ScrollState dirScroll;
extern ScrollState selectedScroll;
extern ScrollState titleScroll;

// Volume
extern int volume;

// User stopped
extern bool userStopped;

// Audio task
extern TaskHandle_t audioTaskHandle;
extern volatile bool stopAudio;

// Current directory
extern String currentDir;

// Menu layout
extern const uint8_t maxViewableItems;
extern const uint8_t pxPerLn;
extern const uint8_t marginLeft;
extern const uint8_t marginTop;
extern const uint8_t maxLenOfItems;

// Playing page layout
extern const uint8_t paddingLeft;
extern const uint8_t titleY;
extern const uint8_t volY;
extern const uint8_t progressBarLength;
extern const uint8_t progressBarX;
extern const uint8_t progressBarY;
extern const uint8_t pauseBtnY;

// Debounce
extern const int debounceInterval;

// Pins
extern const uint8_t backBtnPin;
extern const uint8_t swPin;
extern const uint8_t dtPin;
extern const uint8_t clkPin;

// Bounce2 buttons
extern Bounce2::Button swRotary;
extern Bounce2::Button backBtn;

// OLED
extern const int DISPLAY_ADDRESS;
extern Adafruit_SSD1306 display;