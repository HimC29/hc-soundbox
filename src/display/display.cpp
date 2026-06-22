#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../globals/globals.h"
#include "../sd/sdGlobals.h"
#include "../scrollText/scrollText.h"
#include "../helpers/helpers.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

static void drawBluetoothIcon(int16_t x, int16_t y) {
    // simple bt rune icon
    display.drawLine(x + 4, y, x + 4, y + 11, SSD1306_WHITE);
    display.drawLine(x + 4, y, x + 9, y + 3, SSD1306_WHITE);
    display.drawLine(x + 4, y + 5, x + 9, y + 3, SSD1306_WHITE);
    display.drawLine(x + 4, y + 6, x + 9, y + 8, SSD1306_WHITE);
    display.drawLine(x + 4, y + 11, x + 9, y + 8, SSD1306_WHITE);
    display.drawLine(x + 4, y + 5, x, y + 3, SSD1306_WHITE);
    display.drawLine(x + 4, y + 6, x, y + 8, SSD1306_WHITE);
}

void drawBluetoothScreen(bool isConnected, int volumePercent) {
    const int vol = constrain(volumePercent, 0, 100);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // top row bt icon and title
    drawBluetoothIcon(2, 1);
    display.setCursor(16, 4);
    display.print("HC SoundBox");
    display.drawLine(0, 16, 127, 16, SSD1306_WHITE);

    // connection state
    display.setCursor(2, 24);
    display.print(isConnected ? "Connected" : "Disconnected");

    // volume, only when connected
    if(isConnected) {
        // volume label and bar
        display.setCursor(2, 44);
        display.print("VOL");
        display.setCursor(104, 44);
        if(vol < 100) display.print(" ");
        if(vol < 10) display.print(" ");
        display.print(vol);

        const int16_t barX = 26;
        const int16_t barY = 44;
        const int16_t barW = 74;
        const int16_t barH = 9;
        display.drawRoundRect(barX, barY, barW, barH, 3, SSD1306_WHITE);
        const int16_t fillW = (barW - 2) * vol / 100;
        if(fillW > 0) display.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, SSD1306_WHITE);
    }

    display.display();
}

void showInsertSdMessage() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.println("Please insert\nthe SD card.");
    display.display();
}

void drawMenu(String title,
             String* items,
             int itemCount,
             int selectedIndex,
             int scrollOffset,
             ScrollState& scroll)
{
    uint8_t x = 0;
    uint8_t y = 9;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);

    if(title.length() > maxLenOfItems) {
        display.print("..." + title.substring(title.length() - (maxLenOfItems - 3), title.length()));
    }
    else {
        display.print(title);
    }

    for(int i = 0; i < maxViewableItems; i++) {
        display.setCursor(x, y);
        display.setTextColor(SSD1306_WHITE);

        if(scrollOffset + i >= itemCount) break;
        if(i + scrollOffset == selectedIndex) {
            display.fillRect(0, y, SCREEN_WIDTH, pxPerLn, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
            display.setCursor(x + marginLeft, y + marginTop);
            String item = items[scrollOffset + i];
            if(item.length() > maxLenOfItems) {
                updateScroll(scroll, item, 200);
                display.print(getScrolledText(scroll, item, maxLenOfItems));
            }
            else {
                display.print(item);
            }
        }
        else {
            display.setCursor(x + marginLeft, y + marginTop);
            String item = items[scrollOffset + i];
            if(item.length() > maxLenOfItems) {
                display.print(item.substring(0, maxLenOfItems - 3));
                display.print("...");
            }
            else {
                display.print(item);
            }
        }

        y += pxPerLn;
    }
    display.display();    
}

void drawFileMenu() {
    bool hasNowPlaying = (songInfo.format != "");
    int fileCount = dirContents.fileCount + (hasNowPlaying ? 1 : 0);
    int scrollOffset = menuState.scrollOffset;
    int selectedIndex = menuState.selectedIndex;

    // number of items to draw
    int visibleCount = min((int)maxViewableItems, fileCount - scrollOffset);
    if (visibleCount < 0) visibleCount = 0;

    String visibleItems[maxViewableItems];
    for(int i = 0; i < visibleCount; i++) {
        int idx = scrollOffset + i;
        if(hasNowPlaying) {
            if(idx == 0) {
                visibleItems[i] = "* Now Playing *";
                continue;
            }
            idx--;
        }
        if(dirContents.isDir[idx]) {
            visibleItems[i] = "> " + dirContents.fileNames[idx];
        }
        else {
            visibleItems[i] = dirContents.fileNames[idx];
        }
    }

    drawMenu(currentDir,
             visibleItems,
             visibleCount,
             selectedIndex - scrollOffset,
             0,
             selectedScroll);
}

void updateSelectedItemDisplay() {
    bool hasNowPlaying = (songInfo.format != "");
    int i = menuState.selectedIndex - menuState.scrollOffset;
    uint8_t y = 9 + (i * pxPerLn);
    
    display.fillRect(0, y, SCREEN_WIDTH, pxPerLn, SSD1306_WHITE);  // white highlight
    display.setTextColor(SSD1306_BLACK);  // black text on white bg
    display.setCursor(marginLeft, y + marginTop);
    
    String item;
    int idx = menuState.selectedIndex;
    if(hasNowPlaying) {
        if(idx == 0) {
            item = "* Now Playing *";
        }
        else {
            idx--;
            item = dirContents.isDir[idx] 
                ? "> " + dirContents.fileNames[idx]
                : dirContents.fileNames[idx];
        }
    }
    else {
        item = dirContents.isDir[idx] 
            ? "> " + dirContents.fileNames[idx]
            : dirContents.fileNames[idx];
    }
    
    if(item.length() > maxLenOfItems) {
        updateScroll(selectedScroll, item, 200);
        display.print(getScrolledText(selectedScroll, item, maxLenOfItems));
    }
    else {
        display.print(item);
    }
    display.display();
}


void updateDirDisplay() {
    display.fillRect(0, 0, SCREEN_WIDTH, 9, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    if(currentDir.length() > maxLenOfItems) {
        updateScroll(dirScroll, currentDir, 300);
        display.print(getScrolledText(dirScroll, currentDir, maxLenOfItems));
    }
    else {
        display.print(currentDir);
    }
    display.display();
}

void drawPlayingPage() {
    String title = songInfo.name;

    display.clearDisplay();
    
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(paddingLeft, titleY);
    display.print(title.substring(0, maxLenOfItems));

    display.setCursor(paddingLeft, volY);
    display.print("Volume: ");
    display.print(volume);

    display.drawFastHLine(progressBarX, progressBarY, progressBarLength, SSD1306_WHITE);
    display.fillCircle(progressBarX, progressBarY, 2, SSD1306_WHITE);

    display.display();
}

void updateTitleDisplay() {
    if(songInfo.name.length() > maxLenOfItems) {
        display.fillRect(0, titleY, SCREEN_WIDTH, 10, SSD1306_BLACK);
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(paddingLeft, titleY);
        updateScroll(titleScroll, songInfo.name, 200);
        display.print(getScrolledText(titleScroll, songInfo.name, maxLenOfItems));
    }
}

void updateVolumeDisplay() {
    display.fillRect(0, volY, SCREEN_WIDTH, 10, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(paddingLeft, volY);
    display.print("Volume: ");
    display.print(volume);
}

void updateProgressBar() {
    unsigned long elapsed = songInfo.paused ? songInfo.pausedAt : (millis() - songInfo.startTime);
    int px = map(
        min(elapsed, songInfo.length),
        0,
        songInfo.length,
        progressBarX,
        progressBarX + progressBarLength
    );

    // clear progress bar strip first to erase old circles
    display.fillRect(progressBarX - 2, progressBarY - 2, progressBarLength + 5, 5, SSD1306_BLACK);

    // redraw the bar line
    display.drawFastHLine(progressBarX, progressBarY, progressBarLength, SSD1306_WHITE);

    // draw new circle indicator
    display.fillCircle(px, progressBarY, 2, SSD1306_WHITE);

    // clear time display text area
    display.fillRect(0, progressBarY + 7, SCREEN_WIDTH / 3, 8, SSD1306_BLACK);

    display.setCursor(6, progressBarY + 7);
    display.print(millisToMinSec(elapsed));
}


void updateLengthDisplay() {
    display.fillRect(SCREEN_WIDTH - 30, progressBarY + 7, 30, 8, SSD1306_BLACK);
    display.setCursor(SCREEN_WIDTH - 30, progressBarY + 7);
    display.print(millisToMinSec(songInfo.length));
}

void drawPauseBtn() {
    display.fillCircle(SCREEN_WIDTH / 2, pauseBtnY, 9, SSD1306_BLACK);
    display.fillRect(SCREEN_WIDTH / 2 - 4, pauseBtnY - 5, 3, 10, SSD1306_WHITE);
    display.fillRect(SCREEN_WIDTH / 2 + 1, pauseBtnY - 5, 3, 10, SSD1306_WHITE);
}

void drawResumeBtn() {
    display.fillCircle(SCREEN_WIDTH / 2, pauseBtnY, 9, SSD1306_BLACK);
    display.fillTriangle(
        SCREEN_WIDTH / 2 - 4, pauseBtnY - 5,  // top left
        SCREEN_WIDTH / 2 - 4, pauseBtnY + 5,  // bottom left
        SCREEN_WIDTH / 2 + 5, pauseBtnY,       // right point
        SSD1306_WHITE
    );
}