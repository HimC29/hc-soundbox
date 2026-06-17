#include <Arduino.h>
#include <SD.h>
#include "../globals/globals.h"

String millisToMinSec(unsigned long milliseconds) {
    long totalSeconds = milliseconds / 1000;

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    String formatted;
    formatted += minutes;
    formatted += ":";
    if(seconds < 10) formatted += "0";
    formatted += seconds;

    return formatted;
}

String getFileFormat(String filename) {
    int dotIndex = filename.lastIndexOf('.');
    
    // No dot found, or dot is the last character
    if (dotIndex == -1 || dotIndex == filename.length() - 1) {
        return "";
    }
    
    return filename.substring(dotIndex + 1);
}

#define SD_CS 5

// Loops until SD can init also shows message to user to insert sd card
void awaitSdInit() {
    SD.end();
    // SD init
    while(!SD.begin(SD_CS)) {
        delay(50);
        yield();
    }
}

// Like awaitSdInit(), but lets user cancel with Back button.
// Returns true when SD is initialized, false when cancelled.
bool awaitSdInitOrBack() {
    SD.end();

    // IMPORTANT: SD.begin() can block long enough to miss a quick tap.
    // We also watch a latched flag set by an interrupt so "tap Back" is reliable.
    backBtnLatched = false;

    while(true) {
        if(backBtnLatched) {
            backBtnLatched = false;
            return false;
        }

        // Keep Bounce2 state reasonably fresh too (used elsewhere).
        backBtn.update();
        if(backBtn.pressed()) {
            backBtnLatched = false;
            return false;
        }

        if(SD.begin(SD_CS)) return true;

        // If the user pressed Back while SD.begin() was blocking,
        // the ISR will have latched it for us.
        if(backBtnLatched) {
            backBtnLatched = false;
            return false;
        }

        delay(10);
        yield();
    }
}

static volatile int rotaryDelta = 0;
static int lastState = 0;
static int8_t accumulator = 0;

static void IRAM_ATTR encoderISR() {
    static const int8_t enc_states[] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
    
    int currentState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    int8_t step = enc_states[(lastState << 2) | currentState];
    lastState = currentState;

    if (step != 0) {
        accumulator += step;
        if (accumulator >= 4) {
            accumulator = 0;
            rotaryDelta = rotaryDelta + 1;
        }
        else if (accumulator <= -4) {
            accumulator = 0;
            rotaryDelta = rotaryDelta - 1;
        }
    }
}

void initRotaryInterrupt() {
    pinMode(clkPin, INPUT_PULLUP);
    pinMode(dtPin, INPUT_PULLUP);
    
    // Read initial state
    lastState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    accumulator = 0;
    
    attachInterrupt(digitalPinToInterrupt(clkPin), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(dtPin), encoderISR, CHANGE);
}

// Function to get current rotary encoder state
int readRotary() {
    if (rotaryDelta == 0) return 0;
    
    noInterrupts();
    int delta = rotaryDelta;
    rotaryDelta = 0;
    interrupts();
    
    if (delta > 0) return 1;
    if (delta < 0) return -1;
    return 0;
}