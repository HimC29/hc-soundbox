#pragma once
#include <Arduino.h>
#include <SD.h>
#include "../globals/globals.h"

String millisToMinSec(unsigned long milliseconds);
String getFileFormat(String filename);
uint32_t parseMP3SampleRate(uint8_t* buffer, size_t bufferSize);
uint32_t parseMP3Bitrate(uint8_t* buffer, size_t bufferSize);
void awaitSdInit();
bool awaitSdInitOrBack();
void initRotaryInterrupt();
int readRotary();
