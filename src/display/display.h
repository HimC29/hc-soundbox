#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "globals/globals.h"

// oled instance
extern Adafruit_SSD1306 display;

// display functions
void showInsertSdMessage();
void drawMenu(String title,
              String* items,
              int itemCount,
              int selectedIndex,
              int scrollOffset,
              ScrollState& scroll);
void drawFileMenu();
void updateSelectedItemDisplay();
void updateDirDisplay();
void drawPlayingPage();
void updateTitleDisplay();
void updateVolumeDisplay();
void updateProgressBar();
void updateLengthDisplay();
void drawPauseBtn();
void drawResumeBtn();