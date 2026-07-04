#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#include "globals/globals.h"
#include "sd/sdGlobals.h"
#include "display/display.h"
#include "helpers/helpers.h"
#include "sd/sdState.h"
#include "sd/sdMenu.h"
#include "sd/sdAudio.h"
#include "bt/bt.h"
#include "rgb/rgb.h"

#include "screensaver/screensaver.h"
#include "games/games.h"
#include "globals/settings.h"
#include "controls/controls.h"




String systemMenuItems[] = {"SD Card", "Bluetooth", "Screensavers", "Games", "Settings"};
const int systemMenuItemCount = 5;
const char* bluetoothName = "HC Soundbox";

void drawSystemMenu();

#if defined(ARDUINO_ARCH_ESP32)
#define HC_ISR_ATTR IRAM_ATTR
#else
#define HC_ISR_ATTR
#endif

static void HC_ISR_ATTR onBackBtnFalling() {
    backBtnLatched = true;
}

void initSdAudioOutput() {
    if(output == nullptr) {
        output = new AudioOutputI2S();
    }
    output->SetPinout(bclkPin, lrcPin, dinPin);
    output->SetGain(volume / 100.0);
}

void releaseSdAudioOutput() {
    if(output != nullptr) {
        delete output;
        output = nullptr;
    }
}


void handleSystemMenuSelect() {
    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        updateMenuState(systemMenuState, systemMenuItemCount, rotaryReadings);
        drawMenu("System Menu", systemMenuItems, systemMenuItemCount, systemMenuState.selectedIndex, 0, selectedScroll);
    }

    swRotary.update();
    if(swRotary.pressed()) {
        backBtnLatched = false; // clear stale back button latch
        switch(systemMenuState.selectedIndex) {
            case 0:
                appMode = MODE_SD;
                if (songInfo.format != "") {
                    // song is already playing, keep state and redraw screen
                    if (sdShowingPlayingPage) {
                        drawPlayingPage();
                        updateLengthDisplay();
                        updateProgressBar();
                        if (songInfo.paused) drawResumeBtn();
                        else drawPauseBtn();
                        display.display();
                    } else {
                        drawFileMenu();
                    }
                } else {
                    // fresh entry to sd mode, do full setup
                    volume = Settings::defaultSdVol;
                    initSdAudioOutput();
                    currentDir = "/";
                    while(true) {
                        showInsertSdMessage();
                        // let user go back to system menu while waiting for sd card
                        if(!awaitSdInitOrBack()) {
                            releaseSdAudioOutput();
                            appMode = MODE_SELECT;
                            drawSystemMenu();
                            return;
                        }
                        if(updateDirContents(currentDir.c_str())) break;
                    }
                    menuState.selectedIndex = 0;
                    menuState.scrollOffset = 0;
                    drawFileMenu();
                }
                break;
            case 1:
                appMode = MODE_BLUETOOTH;
                volume = Settings::defaultBtVol;
                stopSong();
                releaseSdAudioOutput();
                startBluetoothMode(bluetoothName);
                break;
            case 2:
                appMode = MODE_SCREENSAVER;
                if (songInfo.format == "") {
                    releaseSdAudioOutput();
                }
                initScreensavers();
                break;
            case 3:
                appMode = MODE_GAMES;
                if (songInfo.format == "") {
                    releaseSdAudioOutput();
                }
                initGames();
                break;

            case 4:
                appMode = MODE_CONTROLS;
                initControlsMenu();
                break;
        }
    }
}

void drawSystemMenu() {
    drawMenu("System Menu", systemMenuItems, systemMenuItemCount, systemMenuState.selectedIndex, 0, selectedScroll);
    setRgbWhite();
}

void setup() {
    Serial.begin(115200);
    initRgb();

    Settings::load();

    initRotaryInterrupt();
    swRotary.attach(swPin, INPUT);
    swRotary.interval(debounceInterval);

    backBtn.attach(backBtnPin, INPUT_PULLUP);
    backBtn.interval(debounceInterval);
    attachInterrupt(digitalPinToInterrupt(backBtnPin), onBackBtnFalling, FALLING);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED failed");
        while (true);
    }
    Wire.setClock(400000);
    display.clearDisplay();
    display.display();

    Settings::applyBrightness();
    Settings::applyLedBrightness();
    drawSystemMenu();
}

void loop() {
    checkAudioStatus();
    switch(appMode) {

        case MODE_SELECT:
            setRgbWhite();
            handleSystemMenuSelect();
            break;

        case MODE_SD: {
            const bool isPlaying = (songInfo.format != "" && !songInfo.paused && !stopAudio);
            if(isPlaying) {
                setRgbRainbow(true);
                updateRgb();
            } else {
                setRgbRainbow(false);
                setRgbPurple();
            }

            if(handleSdMode()) {
                setRgbRainbow(false);
                setRgbWhite();
                drawSystemMenu();
                appMode = MODE_SELECT;
            }
            break;
        }

        case MODE_BLUETOOTH:
            setRgbRainbow(false);
            if(isBluetoothConnected()) {
                setRgbBlue();
            } else {
                setRgbRed();
            }

            if(handleBluetoothMode()) {
                initSdAudioOutput();
                setRgbWhite();
                drawSystemMenu();
                appMode = MODE_SELECT;
            }
            break;

        case MODE_SCREENSAVER:
            if(handleScreensavers()) {
                setRgbWhite();
                drawSystemMenu();
                appMode = MODE_SELECT;
            }
            break;

        case MODE_GAMES:
            if(handleGamesMode()) {
                setRgbWhite();
                drawSystemMenu();
                appMode = MODE_SELECT;
            }
            break;

        case MODE_CONTROLS:
            if(handleControlsMode()) {
                setRgbWhite();
                drawSystemMenu();
                appMode = MODE_SELECT;
            }
            break;
    }
    delay(1);
}
