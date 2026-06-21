#pragma once
#include <Arduino.h>

unsigned long getWAVLength(File file);
unsigned long getMP3Length(File file);
unsigned long getAudioLength(String fileLocation);
void audioTask(void* param);
void handleStartSong(String fileLocation, String fileName, String type, bool showPlayingPage = true);
void handlePause();
void handleResume();
void stopSong();

