#pragma once
#include <Arduino.h>

void initRgb();
void setRgbWhite();
void setRgbRed();
void setRgbGreen();
void setRgbPurple();
void setRgbBlue();
void setRgbRainbow(bool enabled);
void setRgbColor(uint8_t r, uint8_t g, uint8_t b);
void updateRgb();
void applyLedBrightness();

