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
    while(!SD.begin(SD_CS));
}

// Function to get current rotary encoder state
int readRotary() {
    static int lastState = 0;
    static int8_t enc_states[] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
    static int8_t accumulator = 0;

    int currentState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    accumulator += enc_states[(lastState << 2) | currentState];
    lastState = currentState;

    if (accumulator >= 4) { accumulator = 0; return 1; }
    if (accumulator <= -4) { accumulator = 0; return -1; }
    return 0;
}