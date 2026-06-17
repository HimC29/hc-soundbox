#pragma once
#include <Arduino.h>
#include <AudioOutputI2S.h>
#include <Bounce2.h>
#include <Adafruit_SSD1306.h>

extern AudioOutputI2S* output;

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

// Volume
extern int volume;

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

// Latched back button press (set from interrupt) so presses can't be missed
// during long/blocking operations (e.g. SD.begin()).
extern volatile bool backBtnLatched;

// OLED
extern Adafruit_SSD1306 display;
