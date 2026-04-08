#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#include "globals/globals.h"
#include "display/display.h"
#include "stateUpdate/stateUpdate.h"
#include "helpers/helpers.h"
#include "audio/audio.h"

void handleSongPicker() {
    static unsigned long lastMenuDraw = 0;

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        updateMenuState(rotaryReadings);
        drawFileMenu();
    }

    swRotary.update();
    if(swRotary.pressed()) {
        if(dirContents.isDir[menuState.selectedIndex]) {
            String newDir;
            if(currentDir == "/") {
                newDir = currentDir + dirContents.fileNames[menuState.selectedIndex];
            }
            else {
                newDir = currentDir + "/" + dirContents.fileNames[menuState.selectedIndex];
            }
            
            delete[] dirContents.fileNames;
            delete[] dirContents.isDir;

            currentDir = newDir;

            menuState.selectedIndex = 0;
            menuState.scrollOffset = 0;

            while(!updateDirContents(newDir.c_str())) {
                showInsertSdMessage();
                awaitSdInit();
            }
            drawFileMenu();
        }
        else {
            String fileName = dirContents.fileNames[menuState.selectedIndex];
            
            String fileFormat = getFileFormat(fileName);
            fileFormat.toLowerCase();

            String fileLocation;
            if(currentDir == "/") {
                fileLocation = currentDir + fileName;
            }
            else {
                fileLocation = currentDir + "/" + fileName;
            }

            if(fileFormat == "mp3" || fileFormat == "wav") {
                handleStartSong(fileLocation, fileName, fileFormat);
                drawPauseBtn();
                display.display();
                return;
            }
        }
    }

    backBtn.update();
    if(backBtn.pressed()) {
        if(currentDir != "/") {
            String newDir = currentDir.substring(0, currentDir.lastIndexOf('/'));
            if(newDir == "") {
                newDir = "/";
            }

            delete[] dirContents.fileNames;
            delete[] dirContents.isDir;

            currentDir = newDir;

            menuState.selectedIndex = 0;
            menuState.scrollOffset = 0;

            while(!updateDirContents(newDir.c_str())) {
                showInsertSdMessage();
                awaitSdInit();
            }
            drawFileMenu();
        }
    }
    if(millis() - lastMenuDraw >= 200) {
        updateSelectedItemDisplay();
        updateDirDisplay();
        lastMenuDraw = millis();
    }
}

void handlePlayingPage() {
    static unsigned long lastDraw = 0;

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        if(rotaryReadings == 1) {
            if(volume != 100) {
                volume = min(100, volume + 2);
            }
        }
        else if(rotaryReadings == -1) {
            if(volume != 0) {
                volume = max(0, volume - 2);
            }
        }
        output->SetGain(volume / 100.0);
        updateVolumeDisplay();
        display.display();
    }

    // Redraw every 200ms only so that buttons and rotary is more responsive
    if(millis() - lastDraw >= 200) {
        updateTitleDisplay();
        if(!songInfo.paused) {
            updateProgressBar();
        }
        display.display();
        lastDraw = millis();
    }

    backBtn.update();
    if(backBtn.pressed()) {
        if(songInfo.paused) {
            // No task running, clean up directly
            songInfo.format = "";
            songInfo.paused = false;
            stopAudio = false;
            drawFileMenu();
        }
        else {
            userStopped = true;
            stopAudio = true;
        }
    }

    if(audioTaskHandle == NULL && stopAudio && !songInfo.paused) {
        songInfo.format = "";
        stopAudio = false;

        if(userStopped) {
            userStopped = false;
            drawFileMenu();
        }
        else {
            String fileName;
            if(menuState.selectedIndex + 1 == dirContents.fileCount) {
                fileName = dirContents.fileNames[0];
                menuState.selectedIndex = 0;
            }
            else {
                fileName = dirContents.fileNames[menuState.selectedIndex + 1];
                menuState.selectedIndex++;
            }

            String fileFormat = getFileFormat(fileName);

            String fileLocation;
            if(currentDir == "/") {
                fileLocation = currentDir + fileName;
            }
            else {
                fileLocation = currentDir + "/" + fileName;
            }

            if(fileFormat == "mp3" || fileFormat == "wav") {
                handleStartSong(fileLocation, fileName, fileFormat);
                drawPauseBtn();
                display.display();
                return;
            }
            else {
                drawFileMenu();
            }
        }
    }

    swRotary.update();
    if(swRotary.pressed()) {
        if(songInfo.paused) {
            handleResume();
            drawPauseBtn();
        }
        else {
            handlePause();
            drawResumeBtn();
        }
    }
}

void setup() {
    Serial.begin(115200);

    // Rotary Encoder pinModes
    pinMode(clkPin, INPUT_PULLUP);
    pinMode(dtPin, INPUT_PULLUP);
    swRotary.attach(swPin, INPUT);
    swRotary.interval(debounceInterval);

    // Btns pinModes
    backBtn.attach(backBtnPin, INPUT_PULLUP);
    backBtn.interval(debounceInterval);

    // Set output
    output = new AudioOutputI2S();
    output->SetPinout(26, 25, 27);
    output->SetGain(volume / 100.0);

    // OLED init
    Wire.setClock(400000);
    if(!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println("OLED failed");
        while (true);
    }
    display.clearDisplay();
    display.display();

    while(true) {
        showInsertSdMessage();
        awaitSdInit();
        if(updateDirContents(currentDir.c_str())) break;
    }

    Serial.println("Ready!");

    menuState.selectedIndex = 0;
    menuState.scrollOffset = 0;

    // Open root directory at start
    // (user can navigate to other dirs)
    drawFileMenu();
}

void loop() {
    if(songInfo.format == "") {
        handleSongPicker();
    }
    else {
        handlePlayingPage();
    }
}