#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "globals/globals.h"
#include "scrollText/scrollText.h"
#include "helpers/helpers.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

extern ScrollState scrollState;

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
    String* fileNames = dirContents.fileNames;
    int fileCount = dirContents.fileCount;
    
    String* formattedFileNames = new String[fileCount];
    for(int i = 0; i < fileCount; i++) {
        if(dirContents.isDir[i]) {
            formattedFileNames[i] = "> " + fileNames[i];
        }
        else {
            formattedFileNames[i] = fileNames[i];
        }
    }

    drawMenu(currentDir,
             formattedFileNames,
             fileCount,
             menuState.selectedIndex,
             menuState.scrollOffset,
             selectedScroll);

    delete[] formattedFileNames;
}

void updateSelectedItemDisplay() {
    int i = menuState.selectedIndex - menuState.scrollOffset;
    uint8_t y = 9 + (i * pxPerLn);
    
    display.fillRect(0, y, SCREEN_WIDTH, pxPerLn, SSD1306_WHITE);  // white highlight
    display.setTextColor(SSD1306_BLACK);  // black text on white bg
    display.setCursor(marginLeft, y + marginTop);
    
    String item = dirContents.isDir[menuState.selectedIndex] 
        ? "> " + dirContents.fileNames[menuState.selectedIndex]
        : dirContents.fileNames[menuState.selectedIndex];
    
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
    display.drawFastHLine(progressBarX, progressBarY, progressBarLength, SSD1306_WHITE);
    display.fillCircle(
        map(
            min(millis() - songInfo.startTime, songInfo.length),
            0,
            songInfo.length,
            progressBarX,
            progressBarX + progressBarLength
        ),
        progressBarY, 2, SSD1306_WHITE
    );

    display.fillRect(0, progressBarY + 7, SCREEN_WIDTH / 3, 8, SSD1306_BLACK);

    display.setCursor(6, progressBarY + 7);
    display.print(millisToMinSec(millis() - songInfo.startTime));
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