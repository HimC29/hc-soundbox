#pragma once

#include <Arduino.h>

enum ScreensaverType {
    SS_DVD = 0,
    SS_MATRIX = 1,
    SS_OSCILLOSCOPE = 2,
    SS_COUNT = 3
};

extern ScreensaverType currentScreensaver;

void initScreensavers();
bool handleScreensavers();
