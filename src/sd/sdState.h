#pragma once
#include <Arduino.h>

bool readDirContents(const char* workingDirName, DirContents& dest);
bool updateDirContents(const char* workingDirName);
void updateMenuState(MenuState& state, int itemCount, int rotaryDirection);
