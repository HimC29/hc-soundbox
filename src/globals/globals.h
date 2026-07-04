#pragma once
#include <Arduino.h>
#include <AudioOutputI2S.h>
#include <Bounce2.h>
#include <Adafruit_SSD1306.h>

extern AudioOutputI2S* output;

enum AppMode { MODE_SELECT, MODE_SD, MODE_BLUETOOTH, MODE_SCREENSAVER, MODE_GAMES, MODE_CONTROLS };
extern AppMode appMode;


struct MenuState {
    int selectedIndex;
    int scrollOffset;
};
extern MenuState menuState;
extern MenuState systemMenuState;

struct ScrollState {
    int offset;
    unsigned long lastScrollTime;
};
extern ScrollState dirScroll;
extern ScrollState selectedScroll;
extern ScrollState titleScroll;

// volume
extern int volume;

// menu layout
extern const uint8_t maxViewableItems;
extern const uint8_t pxPerLn;
extern const uint8_t marginLeft;
extern const uint8_t marginTop;
extern const uint8_t maxLenOfItems;

// playing page layout
extern const uint8_t paddingLeft;
extern const uint8_t titleY;
extern const uint8_t volY;
extern const uint8_t progressBarLength;
extern const uint8_t progressBarX;
extern const uint8_t progressBarY;
extern const uint8_t pauseBtnY;

// debounce
extern const int debounceInterval;

// pins
extern const uint8_t backBtnPin;
extern const uint8_t swPin;
extern const uint8_t dtPin;
extern const uint8_t clkPin;

extern const uint8_t bclkPin;
extern const uint8_t lrcPin;
extern const uint8_t dinPin;

// bounce2 buttons
extern Bounce2::Button swRotary;
extern Bounce2::Button backBtn;

// latched back button press (set from interrupt) so presses can't be missed
// during long/blocking operations (e.g. sd.begin())
extern volatile bool backBtnLatched;

// oled
extern Adafruit_SSD1306 display;
