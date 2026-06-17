#include <vector>
#include "sdGlobals.h"
#include "sdState.h"
#include "../globals/globals.h"

void updateMenuState(MenuState& state, int itemCount, int rotaryDirection) {
    if(rotaryDirection > 0) {
        for (int i = 0; i < rotaryDirection; i++) {
            if(state.selectedIndex != itemCount - 1) {
                state.selectedIndex++;
            }
            else {
                state.selectedIndex = 0;
            }
        }
    }
    else if(rotaryDirection < 0) {
        for (int i = 0; i < -rotaryDirection; i++) {
            if(state.selectedIndex != 0) {
                state.selectedIndex--;
            }
            else {
                state.selectedIndex = itemCount - 1;
            }
        }
    }

    if(state.selectedIndex < state.scrollOffset) {
        state.scrollOffset = state.selectedIndex;
    }
    if(state.selectedIndex >= state.scrollOffset + maxViewableItems) {
        state.scrollOffset = state.selectedIndex - maxViewableItems + 1;
    }

    selectedScroll.offset = 0;
}

bool updateDirContents(const char* workingDirName) {
    File workingDir = SD.open(workingDirName);

    if(!workingDir) {
        return false;
    }
    
    dirContents.fileNames.clear();
    dirContents.isDir.clear();

    while(true) {
        File entry = workingDir.openNextFile();
        if(!entry) break;
        dirContents.fileNames.push_back(entry.name());
        dirContents.isDir.push_back(entry.isDirectory());
    }

    dirContents.fileCount = dirContents.fileNames.size();

    return true;
}
