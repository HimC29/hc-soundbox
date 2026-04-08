#include "globals.h"
#include <Wire.h>

// Audio
AudioFileSourceSD* source = nullptr;
AudioGeneratorMP3* mp3Decoder = nullptr;
AudioGeneratorWAV* wavDecoder = nullptr;
AudioOutputI2S* output = nullptr;

// Structs
DirContents dirContents = {nullptr, 0, nullptr};
MenuState menuState = {0, 0};
SongInfo songInfo = {"", "", "", 0, 0, false, 0, 0};
ScrollState dirScroll = {0, 0};
ScrollState selectedScroll = {0, 0};
ScrollState titleScroll = {0, 0};

// Volume
int volume = 10;

// User stopped
bool userStopped = false;

// Audio task
TaskHandle_t audioTaskHandle = NULL;
volatile bool stopAudio = false;

// Current directory
String currentDir = "/";

// Menu layout
const uint8_t maxViewableItems = 5;
const uint8_t pxPerLn = 11;
const uint8_t marginLeft = 2;
const uint8_t marginTop = 2;
const uint8_t maxLenOfItems = 20;

// Playing page layout
const uint8_t paddingLeft = 2;
const uint8_t titleY = 2;
const uint8_t volY = 15;
const uint8_t progressBarLength = 100;
const uint8_t progressBarX = 14;
const uint8_t progressBarY = 34;
const uint8_t pauseBtnY = 51;

// Debounce
const int debounceInterval = 100;

// Pins
const uint8_t backBtnPin = 4;
const uint8_t swPin = 34;
const uint8_t dtPin = 33;
const uint8_t clkPin = 32;

// Bounce2 buttons
Bounce2::Button swRotary;
Bounce2::Button backBtn;

// OLED
const int DISPLAY_ADDRESS = 0x3C; // CHANGE THIS IF YOUR ADDRESS IS NOT 0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);