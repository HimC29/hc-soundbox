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
#include "bt/bt.h"
#include "rgb/rgb.h"

#include "screensaver/screensaver.h"
#include "games/games.h"

enum AppMode { MODE_SELECT, MODE_SD, MODE_BLUETOOTH, MODE_SCREENSAVER, MODE_GAMES };
AppMode appMode = MODE_SELECT;

String modeItems[] = {"SD Card", "Bluetooth", "Screensavers", "Games"};
const int modeItemCount = 4;
const char* bluetoothName = "HC Soundbox";

void drawModeMenu();

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
    output->SetPinout(26, 25, 27);
    output->SetGain(volume / 100.0);
}

void releaseSdAudioOutput() {
    if(output != nullptr) {
        delete output;
        output = nullptr;
    }
}


void handleModeSelect() {
    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        updateMenuState(modeMenuState, modeItemCount, rotaryReadings);
        drawMenu("Select Mode", modeItems, modeItemCount, modeMenuState.selectedIndex, 0, selectedScroll);
    }

    swRotary.update();
    if(swRotary.pressed()) {
        if(modeMenuState.selectedIndex == 0) {
            appMode = MODE_SD;
            initSdAudioOutput();
            currentDir = "/";
            while(true) {
                showInsertSdMessage();
                // Allow user to go back to the mode menu while waiting for SD.
                if(!awaitSdInitOrBack()) {
                    releaseSdAudioOutput();
                    appMode = MODE_SELECT;
                    drawModeMenu();
                    return;
                }
                if(updateDirContents(currentDir.c_str())) break;
            }
            menuState.selectedIndex = 0;
            menuState.scrollOffset = 0;
            drawFileMenu();
        }
        else if(modeMenuState.selectedIndex == 1) {
            appMode = MODE_BLUETOOTH;
            releaseSdAudioOutput();
            startBluetoothMode(bluetoothName);
        }
        else if(modeMenuState.selectedIndex == 2) {
            appMode = MODE_SCREENSAVER;
            releaseSdAudioOutput();
            initScreensavers();
        }
        else if(modeMenuState.selectedIndex == 3) {
            appMode = MODE_GAMES;
            releaseSdAudioOutput();
            initGames();
        }
    }
}

void drawModeMenu() {
    drawMenu("Select Mode", modeItems, modeItemCount, modeMenuState.selectedIndex, 0, selectedScroll);
    setRgbWhite();
}

void setup() {
    Serial.begin(115200);
    initRgb();

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

    drawModeMenu();
}

void loop() {
    if(appMode == MODE_SELECT) {
        setRgbWhite();
        handleModeSelect();
    }
    else if(appMode == MODE_SD) {
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
            drawModeMenu();
            appMode = MODE_SELECT;
        }
    }
    else if(appMode == MODE_BLUETOOTH) {
        setRgbRainbow(false);
        if(isBluetoothConnected()) setRgbBlue();
        else setRgbRed();

        if(handleBluetoothMode()) {
            initSdAudioOutput();
            setRgbWhite();
            drawModeMenu();
            appMode = MODE_SELECT;
        }
    }
    else if(appMode == MODE_SCREENSAVER) {
        if(handleScreensavers()) {
            setRgbWhite();
            drawModeMenu();
            appMode = MODE_SELECT;
        }
    }
    else if(appMode == MODE_GAMES) {
        if(handleGamesMode()) {
            setRgbWhite();
            drawModeMenu();
            appMode = MODE_SELECT;
        }
    }
    delay(1);
}
