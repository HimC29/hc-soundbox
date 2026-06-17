#include "sdMenu.h"
#include "sdGlobals.h"
#include "sdAudio.h"
#include "sdState.h"
#include <SD.h>
#include "../globals/globals.h"
#include "../display/display.h"
#include "../helpers/helpers.h"

bool handleSongPicker() {
    static unsigned long lastMenuDraw = 0;

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        updateMenuState(menuState, dirContents.fileCount, rotaryReadings);
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
            
            currentDir = newDir;

            menuState.selectedIndex = 0;
            menuState.scrollOffset = 0;

            while(!updateDirContents(newDir.c_str())) {
                showInsertSdMessage();
                if(!awaitSdInitOrBack()) {
                    return true;
                }
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
                return false;
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

            currentDir = newDir;

            menuState.selectedIndex = 0;
            menuState.scrollOffset = 0;

            while(!updateDirContents(newDir.c_str())) {
                showInsertSdMessage();
                if(!awaitSdInitOrBack()) {
                    return true;
                }
            }
            drawFileMenu();
        }
        else {
            return true;
        }
    }
    if(millis() - lastMenuDraw >= 200) {
        updateSelectedItemDisplay();
        updateDirDisplay();
        lastMenuDraw = millis();
    }
    return false;
}

void handlePlayingPage() {
    static unsigned long lastDraw = 0;

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        if(rotaryReadings > 0) {
            volume = min(100, volume + rotaryReadings * 2);
        }
        else if(rotaryReadings < 0) {
            volume = max(0, volume + rotaryReadings * 2);
        }
        output->SetGain(volume / 100.0);
        updateVolumeDisplay();
        display.display();
    }

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
        userStopped = true;
        stopAudio = true;
        songInfo.paused = false; // ensure task loop unblocks and terminates
        if(output) output->stop(); // Force I2S output to stop so WAV decoder loop unblocks
    }

    if(audioTaskHandle == NULL && stopAudio && !songInfo.paused) {
        songInfo.format = "";
        stopAudio = false;

        if(sdCardRemoved) {
            // Check if card is actually removed or if it was just a natural EOF
            File root = SD.open("/");
            if(root) {
                root.close();
                sdCardRemoved = false;
            }
        }

        if(sdCardRemoved) {
            // Card was pulled - bail out to system menu via handleSdMode return value
            return;
        }
        else if(userStopped) {
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
            // Keep the highlighted item visible when returning to the file list
            // after auto-advancing through tracks.
            if(menuState.selectedIndex < menuState.scrollOffset) {
                menuState.scrollOffset = menuState.selectedIndex;
            }
            if(menuState.selectedIndex >= menuState.scrollOffset + maxViewableItems) {
                menuState.scrollOffset = menuState.selectedIndex - maxViewableItems + 1;
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

bool handleSdMode() {
    if(songInfo.format == "") {
        if(sdCardRemoved) {
            sdCardRemoved = false;
            return true;
        }
        return handleSongPicker();
    }
    else handlePlayingPage();
    return false;
}