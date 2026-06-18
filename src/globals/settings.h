#pragma once
#include <Arduino.h>

namespace Settings {
    extern int brightness;
    extern int ledBrightness;
    extern int volumeStep;
    extern int defaultSdVol;
    extern int defaultBtVol;
    extern int songSorting;

    void load();
    void save();
    void applyBrightness();
    void applyLedBrightness();
    void restoreDefaults();
}
