#include "globals.h"
#include <Wire.h>

AudioOutputI2S* output = nullptr;

MenuState menuState = {0, 0};
MenuState modeMenuState = {0, 0};

ScrollState dirScroll = {0, 0};
ScrollState selectedScroll = {0, 0};
ScrollState titleScroll = {0, 0};

int volume = 10;

const uint8_t maxViewableItems = 5;
const uint8_t pxPerLn = 11;
const uint8_t marginLeft = 2;
const uint8_t marginTop = 2;
const uint8_t maxLenOfItems = 20;

const uint8_t paddingLeft = 2;
const uint8_t titleY = 2;
const uint8_t volY = 15;
const uint8_t progressBarLength = 100;
const uint8_t progressBarX = 14;
const uint8_t progressBarY = 34;
const uint8_t pauseBtnY = 51;

const int debounceInterval = 15;

const uint8_t backBtnPin = 4;
const uint8_t swPin = 34;
const uint8_t dtPin = 33;
const uint8_t clkPin = 32;

Bounce2::Button swRotary;
Bounce2::Button backBtn;

volatile bool backBtnLatched = false;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
