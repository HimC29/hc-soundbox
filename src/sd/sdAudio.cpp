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
    uint32_t offset = 0;
    uint8_t id3Header[10];
    file.seek(0);
    if (file.read(id3Header, 10) == 10) {
        if (memcmp(id3Header, "ID3", 3) == 0) {
            uint32_t tagSize = ((uint32_t)(id3Header[6] & 0x7F) << 21) |
                               ((uint32_t)(id3Header[7] & 0x7F) << 14) |
                               ((uint32_t)(id3Header[8] & 0x7F) << 7)  |
                               ((uint32_t)(id3Header[9] & 0x7F));
            offset = 10 + tagSize;
            if (id3Header[5] & 0x10) { // Has footer (adds 10 bytes at the end of the tag)
                offset += 10;
            }
        }
    }

    // Read 1000 bytes after the ID3v2 tag
    uint8_t header[1000];
    file.seek(offset);
    size_t bytesRead = file.read(header, 1000);
    if (bytesRead < 32) return 0;

    for(int i = 0; i < (int)(bytesRead - 16); i++) {
        if(memcmp(&header[i], "Xing", 4) == 0 || memcmp(&header[i], "Info", 4) == 0) {
            uint32_t frames = ((uint32_t)header[i+8] << 24) | 
                              ((uint32_t)header[i+9] << 16) | 
                              ((uint32_t)header[i+10] << 8) | 
                              header[i+11];
            
            uint32_t sampleRate = parseMP3SampleRate(header, bytesRead);
            
            float duration = ((float)frames * 1152.0) / (float)sampleRate;
            return (unsigned long)(duration * 1000.0);
        }
        else if(memcmp(&header[i], "VBRI", 4) == 0) {
            uint32_t frames = ((uint32_t)header[i+14] << 24) | 
                              ((uint32_t)header[i+15] << 16) | 
                              ((uint32_t)header[i+16] << 8) | 
                              header[i+17];
            
            uint32_t sampleRate = parseMP3SampleRate(header, bytesRead);
            
            float duration = ((float)frames * 1152.0) / (float)sampleRate;
            return (unsigned long)(duration * 1000.0);
        }
    }

    uint32_t audioSize = file.size();
    if(audioSize > offset) {
        audioSize -= offset;
    }

    uint32_t cbrBitrate = parseMP3Bitrate(header, bytesRead);
    return (unsigned long)((audioSize * 8.0) / (float)cbrBitrate * 1000.0);
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
    Serial.printf("[DBG] audioTask START fmt=%s stopAudio=%d userStopped=%d paused=%d\n",
        songInfo.format.c_str(), (int)stopAudio, (int)userStopped, (int)songInfo.paused);
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
            Serial.printf("[DBG] audioTask: decoder loop() returned false (natural EOF). sdCardRemoved=%d\n", (int)sdCardRemoved);
            if(output) output->SetGain(0);
            stopAudio = true;
        }
        vTaskDelay(1);
    }

    Serial.printf("[DBG] audioTask EXIT loop. running=%d stopAudio=%d userStopped=%d sdCardRemoved=%d\n",
        (int)running, (int)stopAudio, (int)userStopped, (int)sdCardRemoved);

    // Mute output immediately.
    if(output) output->SetGain(0);

    // IMPORTANT: Do NOT call mp3Decoder->stop() / wavDecoder->stop() here.
    if(output && !userStopped) output->flush();
    Serial.println("[DBG] audioTask: flush done");

    if(songInfo.format == "mp3") { delete mp3Decoder; mp3Decoder = nullptr; }
    else                         { delete wavDecoder;  wavDecoder  = nullptr; }
    delete source; source = nullptr;
    Serial.println("[DBG] audioTask: decoders deleted");

    if(!sdCardRemoved && output) output->SetGain(volume / 100.0);

    stopAudio = true;
    audioTaskHandle = NULL;
    audioPaused = false;
    Serial.printf("[DBG] audioTask: set audioTaskHandle=NULL stopAudio=true. About to vTaskDelete.\n");
    vTaskDelete(NULL);
}

void handleStartSong(String fileLocation, String fileName, String type, bool showPlayingPage) {
    startingSong = true; // Block checkAudioStatus from re-entering during stopSong() wait
    Serial.printf("[DBG] handleStartSong ENTER file=%s type=%s showPage=%d\n",
        fileLocation.c_str(), type.c_str(), (int)showPlayingPage);
    stopSong(); // Stop currently playing song first (no-op if already stopped)
    // stopSong() sets userStopped=true internally so the audio task exits cleanly.
    // Clear it now — from this point forward we're starting a NEW song, not
    // responding to a user stop. Leaving it set would cause the next
    // checkAudioStatus() to take the userStopped branch instead of auto-advancing.
    userStopped = false;
    Serial.printf("[DBG] handleStartSong: after stopSong. audioTaskHandle=%p userStopped=%d\n",
        (void*)audioTaskHandle, (int)userStopped);

    audioPaused = false;
    songInfo.paused = false; // Always start the new song unpaused
    songInfo.length = getAudioLength(fileLocation);
    source = new AudioFileSourceSD(fileLocation.c_str());

    if(type == "mp3") {
        mp3Decoder = new AudioGeneratorMP3();
        // begin() calls output->begin(). Because we no longer call output->stop()
        // between songs, i2sOn is still true and begin() will just reconfigure
        // the sample rate via SetRate() without destroying/recreating the channel.
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
    // Only force the playing page if explicitly requested (user picked a song).
    // Auto-advance passes sdShowingPlayingPage so the user stays in the file
    // browser / system menu / games without being interrupted.
    if(showPlayingPage) {
        sdShowingPlayingPage = true;
    }

    xTaskCreatePinnedToCore(audioTask, "audioTask", 8192, NULL, 1, &audioTaskHandle, 0);
    if (appMode == MODE_SD && sdShowingPlayingPage) {
        drawPlayingPage();
        updateLengthDisplay();
        updateProgressBar();
        drawPauseBtn();
        display.display();
    }
    startingSong = false; // Allow checkAudioStatus to act again
}

void handlePause() {
    songInfo.paused = true;
    audioPaused = true;
    songInfo.pausedAt = millis() - songInfo.startTime;
    if(output) output->SetGain(0);
}

void handleResume() {
    if(output) output->SetGain(volume / 100.0);
    songInfo.startTime = millis() - songInfo.pausedAt;
    audioPaused = false;
    songInfo.paused = false;
}

void stopSong() {
    if (audioTaskHandle != NULL) {
        userStopped = true;
        stopAudio = true;
        songInfo.paused = false;
        // Use audioPaused to make ConsumeSample() return false immediately,
        // unblocking the audio task on Core 0 without calling output->stop()
        // cross-core (which can corrupt the I2S driver and cause crashes).
        audioPaused = true;
        
        // Wait for task to exit and clean itself up (with timeout for safety)
        unsigned long deadline = millis() + 3000;
        while (audioTaskHandle != NULL && millis() < deadline) {
            delay(10);
        }
        // audioPaused is reset to false inside the audio task before vTaskDelete
    }
}

