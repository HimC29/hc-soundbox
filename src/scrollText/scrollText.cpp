#include "../globals/globals.h"
#include "../globals/settings.h"

void updateScroll(ScrollState& state, const String& text, uint16_t scrollDelay) {
    if(millis() - state.lastScrollTime >= scrollDelay) {
        state.offset++;

        if(state.offset >= (int)text.length() + 3) {
            state.offset = 0;
        }

        state.lastScrollTime = millis();
    }
}

String getScrolledText(ScrollState& state, const String& text, uint8_t maxChars) {
    String paddedText = text + "   " + text;
    return paddedText.substring(state.offset, state.offset + maxChars);
}