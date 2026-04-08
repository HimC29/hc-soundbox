#include "../globals/globals.h"

void updateMenuState(int rotaryDirection) {
    // Turn right go next
    if(rotaryDirection == 1) {
        if(menuState.selectedIndex != dirContents.fileCount - 1) {
            menuState.selectedIndex++;
        }
        else {
            menuState.selectedIndex = 0;
        }
    }
    // Turn left go back
    else if(rotaryDirection == -1) {
        if(menuState.selectedIndex != 0) {
            menuState.selectedIndex--;
        }
        else {
            menuState.selectedIndex = dirContents.fileCount - 1;
        }
    }

    // If current selected is less than the scroll
    if(menuState.selectedIndex < menuState.scrollOffset) {
        // Make the scroll to the selected
        menuState.scrollOffset = menuState.selectedIndex;
    }
    // If the selected is more or equal to the scroll
    if(menuState.selectedIndex >= menuState.scrollOffset + maxViewableItems) {
        // Make the scroll to current selected minus amount of viewable files plus 1
        menuState.scrollOffset = menuState.selectedIndex - maxViewableItems + 1;
    }

    selectedScroll.offset = 0;
}

bool updateDirContents(const char* workingDirName) {
    /*
    vector can dynamically grow or shrink size but arrays can't
    but arrays take less ram then vector

    before looping to check how many files are in the dir to set array size
    then I loop again for the names.
    that is slow.

    so now I use vector, then convert to normal array to save ram
    */

    File workingDir = SD.open(workingDirName);

    if(!workingDir) {
        return false;
    }
    
    std::vector<String> fileNamesVec;
    std::vector<bool> isDirVec;

    while(true) {
        File entry = workingDir.openNextFile();
        if(!entry) break;
        fileNamesVec.push_back(entry.name());
        isDirVec.push_back(entry.isDirectory());
    }

    int fileCount = fileNamesVec.size();

    String* fileNames = new String[fileCount];
    bool* isDir = new bool[fileCount];

    for(int i = 0; i < fileCount; i++) {
        fileNames[i] = fileNamesVec[i];
        isDir[i] = isDirVec[i];
    }    
    
    // Updates the global DirContents contents
    dirContents.fileCount = fileCount;
    dirContents.fileNames = fileNames;
    dirContents.isDir = isDir;

    return true;
}