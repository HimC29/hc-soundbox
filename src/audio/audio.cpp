#include <Arduino.h>
#include "globals/globals.h"
#include "../helpers/helpers.h"
#include "display/display.h"

unsigned long getWAVLength(File file) {
    file.seek(12); // skip "RIFF" header
    
    while(file.available()) {
        char chunkId[4];
        file.read((uint8_t*)chunkId, 4);
        
        uint8_t buf[4];
        file.read(buf, 4);
        uint32_t chunkSize = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
        
        if(memcmp(chunkId, "fmt ", 4) == 0) {
            uint8_t fmtBuf[16];
            file.read(fmtBuf, 16);
            uint16_t channels = fmtBuf[2] | (fmtBuf[3] << 8);
            uint32_t sampleRate = fmtBuf[4] | (fmtBuf[5] << 8) | (fmtBuf[6] << 16) | (fmtBuf[7] << 24);
            uint16_t bitsPerSample = fmtBuf[14] | (fmtBuf[15] << 8);
            
            // Find data chunk
            while(file.available()) {
                char dataId[4];
                file.read((uint8_t*)dataId, 4);
                uint8_t sizeBuf[4];
                file.read(sizeBuf, 4);
                uint32_t dataSize = sizeBuf[0] | (sizeBuf[1] << 8) | (sizeBuf[2] << 16) | (sizeBuf[3] << 24);
                
                if(memcmp(dataId, "data", 4) == 0) {
                    return (unsigned long)((float)dataSize / (sampleRate * channels * (bitsPerSample / 8.0)) * 1000);
                }
                file.seek(file.position() + dataSize);
            }
        } else {
            file.seek(file.position() + chunkSize);
        }
    }
    return 0;
}

unsigned long getMP3Length(File file) {
    uint8_t header[200];
    file.seek(0);
    file.read(header, 200);

    // Search for Xing or Info header
    for(int i = 0; i < 196; i++) {
        if(memcmp(&header[i], "Xing", 4) == 0 || memcmp(&header[i], "Info", 4) == 0) {
            uint32_t frames = (header[i+8] << 24) | (header[i+9] << 16) | (header[i+10] << 8) | header[i+11];
            float duration = (frames * 1152.0) / 44100.0;
            return (unsigned long)(duration * 1000);
        }
    }

    // Fallback: estimate from file size assuming 128kbps
    return (unsigned long)((file.size() * 8.0) / 128000.0 * 1000);
}

unsigned long getAudioLength(String fileLocation) {
    File file = SD.open(fileLocation);
    if(!file) return 0;

    String fileFormat = getFileFormat(fileLocation);
    unsigned long length;

    if(fileFormat == "wav") {
        length = getWAVLength(file);
    }
    else if(fileFormat == "mp3") {
        length = getMP3Length(file);
    }

    file.close();
    return length;
}

void audioTask(void* param) {
    bool running = true;
    while(running && !stopAudio) {
        if(songInfo.format == "mp3") running = mp3Decoder->loop();
        else running = wavDecoder->loop();
        vTaskDelay(1);
    }
    if(songInfo.format == "mp3") { mp3Decoder->stop(); delete mp3Decoder; }
    else { wavDecoder->stop(); delete wavDecoder; }
    delete source;
    stopAudio = true;
    audioTaskHandle = NULL;
    vTaskDelete(NULL);
}

void handleStartSong(String fileLocation, String fileName, String type) {
    songInfo.length = getAudioLength(fileLocation);
    source = new AudioFileSourceSD(fileLocation.c_str());

    if(type == "mp3") {
        mp3Decoder = new AudioGeneratorMP3();
        mp3Decoder->begin(source, output);
        songInfo.format = "mp3";
    }
    else if(type == "wav") {
        wavDecoder = new AudioGeneratorWAV();
        wavDecoder->begin(source, output);
        songInfo.format = "wav";
    }

    songInfo.name = fileName;
    songInfo.fileLocation = fileLocation;
    songInfo.startTime = millis();
    stopAudio = false;
    xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 1, &audioTaskHandle, 0);
    drawPlayingPage();
    updateLengthDisplay();
    display.display();
}

void handlePause() {
    songInfo.savedPos = source->getPos();
    stopAudio = true;
    songInfo.paused = true;
    songInfo.pausedAt = millis() - songInfo.startTime;

    while(audioTaskHandle != NULL) {
        vTaskDelay(1);
    }
}

void handleResume() {
    stopAudio = false;
    source = new AudioFileSourceSD(songInfo.fileLocation.c_str());

    if(songInfo.format == "mp3") {
        mp3Decoder = new AudioGeneratorMP3();
        mp3Decoder->begin(source, output);
    }
    else if(songInfo.format == "wav") {
        wavDecoder = new AudioGeneratorWAV();
        wavDecoder->begin(source, output);
    }

    source->seek(songInfo.savedPos, 0);
    
    songInfo.startTime = millis() - songInfo.pausedAt;
    songInfo.paused = false;
    xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 1, &audioTaskHandle, 0);
}