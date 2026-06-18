#include "settings.h"
#include "globals.h"
#include <Preferences.h>
#include "../rgb/rgb.h"

namespace Settings {
    int brightness = 255;
    int ledBrightness = 255;
    int volumeStep = 2;
    int defaultSdVol = 10;
    int defaultBtVol = 40;
    int songSorting = 0;

    void load() {
        Preferences prefs;
        prefs.begin("soundbox", true);
        brightness = prefs.getInt("bright", 255);
        ledBrightness = prefs.getInt("led_bright", 255);
        volumeStep = prefs.getInt("vol_step", 2);
        defaultSdVol = prefs.getInt("sdvol", 10);
        defaultBtVol = prefs.getInt("btvol", 40);
        songSorting = prefs.getInt("sort", 0);
        prefs.end();
    }

    void save() {
        Preferences prefs;
        prefs.begin("soundbox", false);
        prefs.putInt("bright", brightness);
        prefs.putInt("led_bright", ledBrightness);
        prefs.putInt("vol_step", volumeStep);
        prefs.putInt("sdvol", defaultSdVol);
        prefs.putInt("btvol", defaultBtVol);
        prefs.putInt("sort", songSorting);
        prefs.end();
    }

    void applyBrightness() {
        // Contrast command for SSD1306
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(brightness);

        display.ssd1306_command(0xD9); // SSD1306_SETPRECHARGE
        if (brightness < 50) {
            display.ssd1306_command(0x11); // Very dim pre-charge
        } else {
            display.ssd1306_command(0xF1); // Standard pre-charge
        }
    }

    void applyLedBrightness() {
        ::applyLedBrightness();
    }

    void restoreDefaults() {
        brightness = 255;
        ledBrightness = 255;
        volumeStep = 2;
        defaultSdVol = 10;
        defaultBtVol = 40;
        songSorting = 0;
        save();
        applyBrightness();
        applyLedBrightness();
    }
}
