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

// Parses an MP3 header buffer to find the sample rate
uint32_t parseMP3SampleRate(uint8_t* buffer, size_t bufferSize) {
    for (size_t i = 0; i < bufferSize - 4; i++) {
        // Look for the MP3 Sync Word (11 bits set to 1: 0xFF and top 3 bits of next byte)
        if (buffer[i] == 0xFF && (buffer[i+1] & 0xE0) == 0xE0) {
            uint8_t mpegVersion = (buffer[i+1] >> 3) & 0x03; // 3 = MPEG v1, 2 = MPEG v2
            uint8_t sampleRateIdx = (buffer[i+2] >> 2) & 0x03;
            
            if (mpegVersion == 3) { // MPEG 1
                if (sampleRateIdx == 0) return 44100;
                if (sampleRateIdx == 1) return 48000;
                if (sampleRateIdx == 2) return 32000;
            } else if (mpegVersion == 2) { // MPEG 2
                if (sampleRateIdx == 0) return 22050;
                if (sampleRateIdx == 1) return 24000;
                if (sampleRateIdx == 2) return 16000;
            }
            break; // Found sync but unknown version setup
        }
    }
    return 44100; // Standard robust default fallback
}

// Parses an MP3 header buffer to find the CBR bitrate
uint32_t parseMP3Bitrate(uint8_t* buffer, size_t bufferSize) {
    for (size_t i = 0; i < bufferSize - 4; i++) {
        if (buffer[i] == 0xFF && (buffer[i+1] & 0xE0) == 0xE0) {
            uint8_t bitrateIdx = (buffer[i+2] >> 4) & 0x0F;
            const uint32_t bitrates[] = {0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 
                                         112000, 128000, 160000, 192000, 224000, 256000, 320000, 0};
            if (bitrateIdx > 0 && bitrateIdx < 15) {
                return bitrates[bitrateIdx];
            }
            break;
        }
    }
    return 128000; // Safe baseline fallback
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

bool awaitSdInitOrBack() {
    SD.end();

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
    
    return delta;
}
