#pragma once
#include <Arduino.h>
#include <SD.h>
#include "../globals/globals.h"

String millisToMinSec(unsigned long milliseconds);
String getFileFormat(String filename);
void awaitSdInit();
bool awaitSdInitOrBack();
void initRotaryInterrupt();
int readRotary();