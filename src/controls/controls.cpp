#include "controls.h"
#include "../globals/globals.h"
#include "../globals/settings.h"
#include "../display/display.h"
#include "../helpers/helpers.h"
#include "../sd/sdState.h"

static MenuState controlsMenuState = {0, 0};
static bool editMode = false;
static int originalValue = 0;

struct Setting {
    const char* name;
    int minValue;
    int maxValue;
    int step;
    int* valuePtr;
    String (*formatValue)(int val);
    void (*onChange)();
};

// formatting helpers
static String formatPercent(int val) {
    return String(val) + "%";
}

static String formatOnOff(int val) {
    return val ? "ON" : "OFF";
}

static String formatSort(int val) {
    if (val == 0) return "A-Z";
    if (val == 1) return "Z-A";
    return "File Order";
}

static String formatInt(int val) {
    return String(val);
}

// settings items
static Setting settings[] = {
    // display
    {"OLED Bright", 15, 255, 15, &Settings::brightness, formatInt, Settings::applyBrightness},
    {"LED Bright", 0, 255, 15, &Settings::ledBrightness, formatInt, Settings::applyLedBrightness},
    // audio
    {"Volume Step", 1, 20, 1, &Settings::volumeStep, formatInt, nullptr},
    {"SD Def Vol", 0, 100, 5, &Settings::defaultSdVol, formatPercent, nullptr},
    {"BT Def Vol", 0, 100, 5, &Settings::defaultBtVol, formatPercent, nullptr},
    // media
    {"Song Sort", 0, 2, 1, &Settings::songSorting, formatSort, nullptr},
    // system
    {"Restore Defaults", 0, 0, 0, nullptr, nullptr, Settings::restoreDefaults}
};
static const int numSettings = sizeof(settings) / sizeof(Setting);

static void updateSettingsMenuStrings(String* items) {
    for (int i = 0; i < numSettings; i++) {
        if (settings[i].valuePtr == nullptr) {
            items[i] = String(settings[i].name);
        } else {
            String valStr = settings[i].formatValue(*(settings[i].valuePtr));
            if (editMode && controlsMenuState.selectedIndex == i) {
                items[i] = String(settings[i].name) + ": <" + valStr + ">";
            } else {
                items[i] = String(settings[i].name) + ": " + valStr;
            }
        }
    }
}

static void drawControlsMenu() {
    String items[numSettings];
    updateSettingsMenuStrings(items);
    drawMenu("Settings", items, numSettings, controlsMenuState.selectedIndex, controlsMenuState.scrollOffset, selectedScroll);
}

void initControlsMenu() {
    controlsMenuState.selectedIndex = 0;
    controlsMenuState.scrollOffset = 0;
    editMode = false;
    drawControlsMenu();
}

bool handleControlsMode() {
    int rotaryReadings = readRotary();

    if (rotaryReadings != 0) {
        if (editMode) {
            // edit mode, change value
            Setting& s = settings[controlsMenuState.selectedIndex];
            if (s.valuePtr != nullptr) {
                int currentVal = *(s.valuePtr);
                currentVal += rotaryReadings * s.step;
                currentVal = constrain(currentVal, s.minValue, s.maxValue);
                *(s.valuePtr) = currentVal;
                if (s.onChange != nullptr) {
                    s.onChange();
                }
                drawControlsMenu();
            }
        } else {
            // scroll mode, scroll through items
            updateMenuState(controlsMenuState, numSettings, rotaryReadings);
            drawControlsMenu();
        }
    }

    swRotary.update();
    if (swRotary.pressed()) {
        Setting& s = settings[controlsMenuState.selectedIndex];
        if (s.valuePtr == nullptr) {
            // trigger action directly
            if (s.onChange != nullptr) {
                s.onChange();
            }
            drawControlsMenu();
        } else {
            if (editMode) {
                // confirm value and exit edit mode
                editMode = false;
                Settings::save();
                drawControlsMenu();
            } else {
                // enter edit mode
                editMode = true;
                originalValue = *(s.valuePtr);
                drawControlsMenu();
            }
        }
    }

    backBtn.update();
    if (backBtn.pressed() || backBtnLatched) {
        backBtnLatched = false;
        if (editMode) {
            // cancel edit mode, restore original value
            Setting& s = settings[controlsMenuState.selectedIndex];
            if (s.valuePtr != nullptr) {
                *(s.valuePtr) = originalValue;
                if (s.onChange != nullptr) {
                    s.onChange();
                }
            }
            editMode = false;
            drawControlsMenu();
        } else {
            // exit settings menu completely
            return true;
        }
    }

    return false;
}
