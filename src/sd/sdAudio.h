#pragma once
#include <Arduino.h>

unsigned long getWAVLength(File& file);
unsigned long getMP3Length(File& file);
unsigned long getAudioLength(const String& fileLocation);
void audioTask(void* param);
void handleStartSong(const String& fileLocation, const String& fileName, const String& type, bool showPlayingPage = true);
void handlePause();
void handleResume();
void stopSong();

