#pragma once
#include <Arduino.h>
#include "globals/globals.h"

bool updateDirContents(const char* workingDirName);
void updateMenuState(int rotaryDirection);