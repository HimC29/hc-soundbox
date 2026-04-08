#pragma once
#include <Arduino.h>
#include "../globals/globals.h"

void updateScroll(ScrollState& state, String& text, uint16_t scrollDelay);
String getScrolledText(ScrollState& state, String& text, uint8_t maxChars);
