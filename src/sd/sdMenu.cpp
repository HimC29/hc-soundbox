#include "sdMenu.h"
#include "sdGlobals.h"
#include "sdAudio.h"
#include "sdState.h"
#include <SD.h>
#include "../globals/globals.h"
#include "../display/display.h"
#include "../helpers/helpers.h"
#include "../globals/settings.h"

bool handleSongPicker() {
    static unsigned long lastMenuDraw = 0;

    bool hasNowPlaying = (songInfo.format != "");
    int totalItems = dirContents.fileCount + (hasNowPlaying ? 1 : 0);

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        updateMenuState(menuState, totalItems, rotaryReadings);
        drawFileMenu();
    }

    swRotary.update();
    if(swRotary.pressed()) {
        int adjustedIndex = menuState.selectedIndex;
        if(hasNowPlaying) {
            if(menuState.selectedIndex == 0) {
                // return to playing page
                sdShowingPlayingPage = true;
                drawPlayingPage();
                updateLengthDisplay();
                updateProgressBar();
                if(songInfo.paused) drawResumeBtn();
                else drawPauseBtn();
                display.display();
                return false;

            }
            adjustedIndex--;
        }

        if(dirContents.isDir[adjustedIndex]) {
            String newDir;
            if(currentDir == "/") {
                newDir = currentDir + dirContents.fileNames[adjustedIndex];
            }
            else {
                newDir = currentDir + "/" + dirContents.fileNames[adjustedIndex];
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
            String fileName = dirContents.fileNames[adjustedIndex];
            
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
                playbackDir = currentDir;
                playbackDirContents = dirContents;
                playbackSelectedIndex = adjustedIndex;
                handleStartSong(fileLocation, fileName, fileFormat);
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
            volume = min(100, volume + rotaryReadings * Settings::volumeStep);
        }
        else if(rotaryReadings < 0) {
            volume = max(0, volume + rotaryReadings * Settings::volumeStep);
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
        unsigned long pressTime = millis();
        bool isLongPress = false;
        while (digitalRead(backBtnPin) == LOW) {
            if (millis() - pressTime > 800) {
                isLongPress = true;
                break;
            }
            delay(10);
        }

        if(isLongPress) {
            userStopped = true;
            stopAudio = true;
            songInfo.paused = false; // ensure task loop unblocks and terminates
            audioPaused = false;
            if(output) output->stop(); // force i2s output to stop so wav decoder loop unblocks
        }
        else {
            // short press: go back to menu but keep playing!
            sdShowingPlayingPage = false;
            drawFileMenu();
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
    if(sdCardRemoved) {
        sdCardRemoved = false;
        return true;
    }
    if(songInfo.format != "" && sdShowingPlayingPage) {
        handlePlayingPage();
    }
    else {
        return handleSongPicker();
    }
    return false;
}

void checkAudioStatus() {
    // guard: handlestartsong sets this while it's running (including during the
    // blocking stopsong() wait). without this, loop() would call checkaudiostatus
    // again mid-stopsong, see audiotaskhandle==null && stopaudio==true, and fire
    // a spurious second auto-advance
    if (startingSong) return;

    static unsigned long lastLog = 0;
    if(stopAudio && millis() - lastLog > 2000) {
        Serial.printf("[DBG] checkAudioStatus poll: handle=%p stopAudio=%d paused=%d userStopped=%d sdCardRemoved=%d fmt='%s'\n",
            (void*)audioTaskHandle, (int)stopAudio, (int)songInfo.paused,
            (int)userStopped, (int)sdCardRemoved, songInfo.format.c_str());
        lastLog = millis();
    }
    if(audioTaskHandle == NULL && stopAudio && !songInfo.paused) {
        Serial.printf("[DBG] checkAudioStatus TRIGGERED: userStopped=%d sdCardRemoved=%d playbackFileCount=%d playbackSelectedIdx=%d\n",
            (int)userStopped, (int)sdCardRemoved, playbackDirContents.fileCount, playbackSelectedIndex);
        songInfo.format = "";
        stopAudio = false;

        if(sdCardRemoved) {
            File root = SD.open("/");
            if(root) {
                root.close();
                Serial.println("[DBG] checkAudioStatus: sdCardRemoved but card IS accessible - clearing flag");
                sdCardRemoved = false;
            }
        }

        if(sdCardRemoved) {
            Serial.println("[DBG] checkAudioStatus: card truly removed - returning");
            return;
        }
        else if(userStopped) {
            Serial.println("[DBG] checkAudioStatus: userStopped branch - no auto-advance");
            userStopped = false;
            if(appMode == MODE_SD && !sdShowingPlayingPage) {
                drawFileMenu();
            }
        }
        else {
            Serial.printf("[DBG] checkAudioStatus: AUTO-ADVANCE branch. playbackFileCount=%d playbackSelectedIdx=%d\n",
                playbackDirContents.fileCount, playbackSelectedIndex);
            if (playbackDirContents.fileCount == 0) {
                Serial.println("[DBG] checkAudioStatus: playbackDirContents.fileCount==0, cannot advance");
                return;
            }

            // find next playable file using our robust loop
            int nextIndex = playbackSelectedIndex;
            bool found = false;
            for (int attempt = 0; attempt < playbackDirContents.fileCount; attempt++) {
                nextIndex = (nextIndex + 1) % playbackDirContents.fileCount;
                if (!playbackDirContents.isDir[nextIndex]) {
                    String fmt = getFileFormat(playbackDirContents.fileNames[nextIndex]);
                    fmt.toLowerCase();
                    if (fmt == "mp3" || fmt == "wav") {
                        found = true;
                        playbackSelectedIndex = nextIndex;
                        break;
                    }
                }
            }

            if (!found) {
                Serial.println("[DBG] checkAudioStatus auto-advance: no playable song found");
                return;
            }

            String fileName = playbackDirContents.fileNames[playbackSelectedIndex];
            String fileFormat = getFileFormat(fileName);
            fileFormat.toLowerCase();

            String fileLocation;
            if(playbackDir == "/") {
                fileLocation = playbackDir + fileName;
            }
            else {
                fileLocation = playbackDir + "/" + fileName;
            }

            Serial.printf("[DBG] checkAudioStatus auto-advance: next='%s' fmt='%s' location='%s'\n",
                fileName.c_str(), fileFormat.c_str(), fileLocation.c_str());

            Serial.printf("[DBG] checkAudioStatus: calling handleStartSong showPlayingPage=%d\n",
                (int)sdShowingPlayingPage);
            handleStartSong(fileLocation, fileName, fileFormat, sdShowingPlayingPage);
            Serial.println("[DBG] checkAudioStatus: handleStartSong returned");

            if (appMode == MODE_SD && !sdShowingPlayingPage) {
                drawFileMenu();
            }
        }
    }
}