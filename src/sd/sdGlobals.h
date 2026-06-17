#pragma once
#include <Arduino.h>
#include <vector>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include "../globals/globals.h"

#define SD_CS 5

extern AudioFileSourceSD* source;
extern AudioGeneratorMP3* mp3Decoder;
extern AudioGeneratorWAV* wavDecoder;

struct DirContents {
    std::vector<String> fileNames;
    int fileCount;
    std::vector<bool> isDir;
};
extern DirContents dirContents;

struct SongInfo {
    String format;
    String name;
    String fileLocation;
    unsigned long length;
    unsigned long startTime;
    bool paused;
    uint32_t savedPos;
    unsigned long pausedAt;
};
extern SongInfo songInfo;

extern bool userStopped;
extern TaskHandle_t audioTaskHandle;
extern volatile bool stopAudio;
extern volatile bool sdCardRemoved;
extern String currentDir;
