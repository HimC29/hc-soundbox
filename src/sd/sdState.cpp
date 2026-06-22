#include <vector>
#include <algorithm>
#include "sdGlobals.h"
#include "sdState.h"
#include "../globals/globals.h"
#include "../globals/settings.h"

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

    struct DirEntry {
        String name;
        bool isDir;
    };
    std::vector<DirEntry> entries;

    while(true) {
        File entry = workingDir.openNextFile();
        if(!entry) break;
        entries.push_back({entry.name(), entry.isDirectory()});
    }
    workingDir.close();

    if (Settings::songSorting == 0 || Settings::songSorting == 1) {
        std::sort(entries.begin(), entries.end(), [](const DirEntry& a, const DirEntry& b) {
            if (a.isDir != b.isDir) {
                return a.isDir; // directories first
            }
            String a_lower = a.name; a_lower.toLowerCase();
            String b_lower = b.name; b_lower.toLowerCase();
            return Settings::songSorting == 0 ? (a_lower < b_lower) : (a_lower > b_lower);
        });
    }

    for (const auto& entry : entries) {
        dirContents.fileNames.push_back(entry.name);
        dirContents.isDir.push_back(entry.isDir);
    }

    dirContents.fileCount = dirContents.fileNames.size();

    return true;
}
