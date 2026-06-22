#include "rgb.h"
#include "../globals/settings.h"

namespace {
constexpr uint8_t RED_PIN = 14;
constexpr uint8_t GREEN_PIN = 15;
constexpr uint8_t BLUE_PIN = 13;
// per-channel calibration (0..255). reduce green because it appears brighter
constexpr uint8_t RED_LEVEL = 140;
constexpr uint8_t GREEN_LEVEL = 45;
constexpr uint8_t BLUE_LEVEL = 255;

bool rainbowEnabled = false;
unsigned long lastStepMs = 0;
uint16_t hue = 0; // 0..359

uint8_t lastR = 255;
uint8_t lastG = 255;
uint8_t lastB = 255;

void writeRgb(uint8_t r, uint8_t g, uint8_t b) {
    lastR = r;
    lastG = g;
    lastB = b;
    const uint8_t rr = (uint32_t)r * RED_LEVEL * Settings::ledBrightness / (255 * 255);
    const uint8_t gg = (uint32_t)g * GREEN_LEVEL * Settings::ledBrightness / (255 * 255);
    const uint8_t bb = (uint32_t)b * BLUE_LEVEL * Settings::ledBrightness / (255 * 255);
    analogWrite(RED_PIN, rr);
    analogWrite(GREEN_PIN, gg);
    analogWrite(BLUE_PIN, bb);
}

// fast integer hsv->rgb for full saturation/value
void hueToRgb(uint16_t h, uint8_t& r, uint8_t& g, uint8_t& b) {
    const uint8_t region = (h / 60) % 6;
    const uint8_t remainder = (h % 60) * 255 / 60;
    const uint8_t q = 255 - remainder;
    const uint8_t t = remainder;

    switch(region) {
        case 0: r = 255; g = t;   b = 0;   break;
        case 1: r = q;   g = 255; b = 0;   break;
        case 2: r = 0;   g = 255; b = t;   break;
        case 3: r = 0;   g = q;   b = 255; break;
        case 4: r = t;   g = 0;   b = 255; break;
        default:r = 255; g = 0;   b = q;   break;
    }
}
} // namespace

void initRgb() {
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    setRgbWhite();
}

void setRgbWhite() {
    rainbowEnabled = false;
    writeRgb(255, 255, 255);
}

void setRgbRed() {
    rainbowEnabled = false;
    writeRgb(255, 0, 0);
}

void setRgbGreen() {
    rainbowEnabled = false;
    writeRgb(0, 255, 0);
}

void setRgbPurple() {
    rainbowEnabled = false;
    writeRgb(255, 0, 255);
}

void setRgbBlue() {
    rainbowEnabled = false;
    writeRgb(0, 0, 255);
}

void setRgbRainbow(bool enabled) {
    const bool wasEnabled = rainbowEnabled;
    rainbowEnabled = enabled;
    if(enabled && !wasEnabled) {
        lastStepMs = millis();
    }
}

void setRgbColor(uint8_t r, uint8_t g, uint8_t b) {
    rainbowEnabled = false;
    writeRgb(r, g, b);
}

void updateRgb() {
    if(!rainbowEnabled) return;

    const unsigned long now = millis();
    if(now - lastStepMs < 20) return; // ~50 updates/sec for smooth transitions
    lastStepMs = now;

    uint8_t r, g, b;
    hueToRgb(hue, r, g, b);
    writeRgb(r, g, b);

    hue = (hue + 1) % 360;
}

void applyLedBrightness() {
    writeRgb(lastR, lastG, lastB);
}
