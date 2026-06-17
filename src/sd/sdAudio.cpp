#include <Arduino.h>
#include "sdGlobals.h"
#include "sdAudio.h"
#include "../globals/globals.h"
#include "../helpers/helpers.h"
#include "../display/display.h"

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

    for(int i = 0; i < 196; i++) {
        if(memcmp(&header[i], "Xing", 4) == 0 || memcmp(&header[i], "Info", 4) == 0) {
            uint32_t frames = (header[i+8] << 24) | (header[i+9] << 16) | (header[i+10] << 8) | header[i+11];
            float duration = (frames * 1152.0) / 44100.0;
            return (unsigned long)(duration * 1000);
        }
    }

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
        if(songInfo.paused) {
            int16_t silence[2] = {0, 0};
            if(!output || !output->ConsumeSample(silence)) {
                vTaskDelay(10);
            }
            continue;
        }
        if(songInfo.format == "mp3") running = mp3Decoder->loop();
        else running = wavDecoder->loop();
        if(!running && !stopAudio && !userStopped) {
            // Decoder stopped on its own without user requesting it.
            // This means the SD card was pulled or a read error occurred.
            // Silence output immediately before the task cleanup to stop glitching.
            output->SetGain(0);
            sdCardRemoved = true;
            stopAudio = true;
        }
        vTaskDelay(1);
    }
    if(output) output->SetGain(0);
    if(songInfo.format == "mp3") { mp3Decoder->stop(); delete mp3Decoder; }
    else { wavDecoder->stop(); delete wavDecoder; }
    delete source;
    if(!sdCardRemoved) output->SetGain(volume / 100.0);
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

    if(output) {
        output->SetGain(volume / 100.0);
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
    songInfo.paused = true;
    songInfo.pausedAt = millis() - songInfo.startTime;
    if(output) output->SetGain(0);
}

void handleResume() {
    if(output) output->SetGain(volume / 100.0);
    songInfo.startTime = millis() - songInfo.pausedAt;
    songInfo.paused = false;
}
