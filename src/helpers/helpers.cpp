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

String getFileFormat(const String& filename) {
    int dotIndex = filename.lastIndexOf('.');
    
    // no dot found, or dot is the last character
    if (dotIndex == -1 || dotIndex == filename.length() - 1) {
        return "";
    }
    
    return filename.substring(dotIndex + 1);
}

// parses an mp3 header buffer to find the sample rate
uint32_t parseMP3SampleRate(uint8_t* buffer, size_t bufferSize) {
    for (size_t i = 0; i < bufferSize - 4; i++) {
        // look for the mp3 sync word (11 bits set to 1: 0xff and top 3 bits of next byte)
        if (buffer[i] == 0xFF && (buffer[i+1] & 0xE0) == 0xE0) {
            uint8_t mpegVersion = (buffer[i+1] >> 3) & 0x03; // 3 = mpeg v1, 2 = mpeg v2
            uint8_t sampleRateIdx = (buffer[i+2] >> 2) & 0x03;
            
            if (mpegVersion == 3) { // mpeg 1
                if (sampleRateIdx == 0) return 44100;
                if (sampleRateIdx == 1) return 48000;
                if (sampleRateIdx == 2) return 32000;
            } else if (mpegVersion == 2) { // mpeg 2
                if (sampleRateIdx == 0) return 22050;
                if (sampleRateIdx == 1) return 24000;
                if (sampleRateIdx == 2) return 16000;
            }
            break; // found sync but unknown version setup
        }
    }
    return 44100; // standard robust default fallback
}

// parses an mp3 header buffer to find the cbr bitrate
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
    return 128000; // safe baseline fallback
}

#define SD_CS 5

// loops until sd can init also shows message to user to insert sd card
void awaitSdInit() {
    SD.end();
    // sd init
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

        // keep bounce2 state reasonably fresh too (used elsewhere)
        backBtn.update();
        if(backBtn.pressed()) {
            backBtnLatched = false;
            return false;
        }

        if(SD.begin(SD_CS)) return true;

        // if the user pressed back while sd.begin() was blocking,
        // the isr will have latched it for us
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
    
    // read initial state
    lastState = (digitalRead(clkPin) << 1) | digitalRead(dtPin);
    accumulator = 0;
    
    attachInterrupt(digitalPinToInterrupt(clkPin), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(dtPin), encoderISR, CHANGE);
}

// function to get current rotary encoder state
int readRotary() {
    if (rotaryDelta == 0) return 0;
    
    noInterrupts();
    int delta = rotaryDelta;
    rotaryDelta = 0;
    interrupts();
    
    return delta;
}
