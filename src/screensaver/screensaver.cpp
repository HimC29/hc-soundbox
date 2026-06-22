#include "screensaver.h"
#include "../globals/globals.h"
#include "../helpers/helpers.h"
#include "../rgb/rgb.h"
#include "../sd/sdGlobals.h"
#include "../sd/sdAudio.h"
#include "../globals/settings.h"


ScreensaverType currentScreensaver = SS_DVD;
static unsigned long lastFrameTime = 0;

// dvd bounce variables
static float dvdX = 10.0f;
static float dvdY = 10.0f;
static float dvdVx = 1.2f;
static float dvdVy = 0.9f;
static constexpr int dvdW = 32;
static constexpr int dvdH = 12;

static const uint8_t dvdColors[][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {255, 0, 255},
    {0, 255, 255},
    {255, 127, 0},
    {128, 0, 255}
};
static constexpr int numDvdColors = sizeof(dvdColors) / sizeof(dvdColors[0]);
static int dvdColorIdx = 0;

// matrix rain variables
struct MatrixColumn {
    int16_t headY;
    int16_t length;
    unsigned long speed;
    unsigned long lastUpdate;
    char chars[8];
};
static MatrixColumn matrixCols[16];

// oscilloscope variables
static float currentAmplitude = 15.0f;
static float targetAmplitude = 15.0f;
static float currentFrequency = 0.05f;
static float targetFrequency = 0.05f;
static float phase = 0.0f;
static constexpr float phaseSpeed = 0.12f;
static unsigned long lastTargetChange = 0;
static unsigned long targetChangeInterval = 4000;

static void initActiveScreensaver() {
    switch (currentScreensaver) {
        case SS_DVD:
            dvdX = random(0, 128 - dvdW);
            dvdY = random(0, 64 - dvdH);
            dvdVx = (random(2) == 0 ? 1.0f : -1.0f) * (0.8f + (random(80) / 100.0f));
            dvdVy = (random(2) == 0 ? 1.0f : -1.0f) * (0.8f + (random(80) / 100.0f));
            dvdColorIdx = random(numDvdColors);
            setRgbColor(dvdColors[dvdColorIdx][0], dvdColors[dvdColorIdx][1], dvdColors[dvdColorIdx][2]);
            break;
            
        case SS_MATRIX:
            for (int c = 0; c < 16; c++) {
                matrixCols[c].headY = -random(2, 15);
                matrixCols[c].length = random(4, 9);
                matrixCols[c].speed = random(60, 180);
                matrixCols[c].lastUpdate = millis();
                for (int r = 0; r < 8; r++) {
                    matrixCols[c].chars[r] = (char)random(33, 126);
                }
            }
            setRgbGreen();
            break;
            
        case SS_OSCILLOSCOPE:
            currentAmplitude = 15.0f;
            targetAmplitude = random(8, 26);
            currentFrequency = 0.05f;
            {
                float cycles = 0.5f + (random(350) / 100.0f);
                targetFrequency = cycles * 2.0f * PI / 128.0f;
            }
            phase = 0.0f;
            lastTargetChange = millis();
            targetChangeInterval = random(3000, 6000);
            setRgbColor(0, 180, 255);
            break;
            
        default:
            break;
    }
}

void initScreensavers() {
    currentScreensaver = SS_DVD;
    initActiveScreensaver();
    lastFrameTime = millis();
}

static void updateAndDrawDvd() {
    dvdX += dvdVx;
    dvdY += dvdVy;
    
    bool collided = false;
    if (dvdX <= 0) {
        dvdX = 0;
        dvdVx = -dvdVx;
        collided = true;
    } else if (dvdX >= 128 - dvdW) {
        dvdX = 128 - dvdW;
        dvdVx = -dvdVx;
        collided = true;
    }
    
    if (dvdY <= 0) {
        dvdY = 0;
        dvdVy = -dvdVy;
        collided = true;
    } else if (dvdY >= 64 - dvdH) {
        dvdY = 64 - dvdH;
        dvdVy = -dvdVy;
        collided = true;
    }
    
    if (collided) {
        dvdColorIdx = (dvdColorIdx + 1) % numDvdColors;
        setRgbColor(dvdColors[dvdColorIdx][0], dvdColors[dvdColorIdx][1], dvdColors[dvdColorIdx][2]);
    }
    
    display.drawRoundRect((int16_t)dvdX, (int16_t)dvdY, dvdW, dvdH, 3, SSD1306_WHITE);
    display.setCursor((int16_t)dvdX + 7, (int16_t)dvdY + 2);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.print("DVD");
}

static void updateAndDrawMatrix() {
    // pulse the green led gently to match the code rain!
    float pulse = 115.0f + 85.0f * sin(millis() / 500.0f);
    setRgbColor(0, (uint8_t)pulse, 0);

    for (int c = 0; c < 16; c++) {
        MatrixColumn& col = matrixCols[c];
        
        if (millis() - col.lastUpdate >= col.speed) {
            col.headY++;
            col.lastUpdate = millis();
            for (int r = 0; r < 8; r++) {
                if (random(100) < 15) {
                    col.chars[r] = (char)random(33, 126);
                }
            }
            
            if (col.headY - col.length >= 8) {
                col.headY = -random(2, 10);
                col.length = random(4, 9);
                col.speed = random(60, 180);
                for (int r = 0; r < 8; r++) {
                    col.chars[r] = (char)random(33, 126);
                }
            }
        }
        
        for (int r = 0; r < 8; r++) {
            if (r <= col.headY && r > col.headY - col.length) {
                char ch = col.chars[r];
                bool isHead = (r == col.headY);
                if (isHead) {
                    display.fillRect(c * 8, r * 8, 8, 8, SSD1306_WHITE);
                    display.setTextColor(SSD1306_BLACK);
                } else {
                    display.setTextColor(SSD1306_WHITE);
                }
                display.setCursor(c * 8 + 1, r * 8);
                display.setTextSize(1);
                display.write(ch);
            }
        }
    }
}

static void updateAndDrawOscilloscope() {
    currentAmplitude += (targetAmplitude - currentAmplitude) * 0.05f;
    currentFrequency += (targetFrequency - currentFrequency) * 0.05f;
    
    phase += phaseSpeed;
    if (phase >= 2.0f * PI) {
        phase -= 2.0f * PI;
    }
    
    if (millis() - lastTargetChange >= targetChangeInterval) {
        targetAmplitude = random(8, 26);
        float cycles = 0.5f + (random(350) / 100.0f);
        targetFrequency = cycles * 2.0f * PI / 128.0f;
        
        targetChangeInterval = random(3000, 6000);
        lastTargetChange = millis();
    }
    
    // pulse cyan/blue phosphor led trace gently based on animation phase
    float ledPulse = 128.0f + 127.0f * sin(phase * 2.0f);
    setRgbColor(0, (uint8_t)(ledPulse * 0.7f), (uint8_t)ledPulse);
    
    float center_y = 32.0f;
    float lastY = center_y + currentAmplitude * sin(currentFrequency * 0 + phase);
    for (int x = 1; x < 128; x++) {
        float y = center_y + currentAmplitude * sin(currentFrequency * (float)x + phase);
        display.drawLine(x - 1, (int16_t)lastY, x, (int16_t)y, SSD1306_WHITE);
        lastY = y;
    }
}

static unsigned long overlayEndTime = 0;
static String lastOverlaySongName = "";
static int lastOverlayVolume = -1;
static bool lastOverlayPaused = false;
static unsigned long lastPressTime = 0;

void drawScreensaverMusicOverlay() {
    if (songInfo.format == "") return;

    // detect track change, volume change, or pause/resume change
    if (songInfo.name != lastOverlaySongName || volume != lastOverlayVolume || songInfo.paused != lastOverlayPaused) {
        overlayEndTime = millis() + 3000;
        lastOverlaySongName = songInfo.name;
        lastOverlayVolume = volume;
        lastOverlayPaused = songInfo.paused;
    }

    if (millis() < overlayEndTime) {
        // draw a sleek black banner at the top
        display.fillRect(0, 0, 128, 14, SSD1306_BLACK);
        display.drawFastHLine(0, 14, 128, SSD1306_WHITE);

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(4, 3);
        
        // draw small note icon or state
        if (songInfo.paused) {
            display.print("|| ");
        } else {
            display.print("> ");
        }
        
        // draw truncated song name
        String nameToShow = songInfo.name;
        if (nameToShow.length() > 14) {
            nameToShow = nameToShow.substring(0, 11) + "...";
        }
        display.print(nameToShow);

        // draw volume percentage
        display.setCursor(94, 3);
        display.print("V:");
        display.print(volume);
    }
}

bool handleScreensavers() {
    int rotaryReadings = readRotary();
    if (rotaryReadings != 0) {
        // button held down (pin dt or button pin? the button pin is swpin!)
        if (digitalRead(swPin) == LOW) {
            if (songInfo.format != "") {
                volume = constrain(volume + rotaryReadings * Settings::volumeStep, 0, 100);
                if (output) output->SetGain(volume / 100.0);
            }
        } else {
            if (rotaryReadings > 0) {
                currentScreensaver = (ScreensaverType)((currentScreensaver + 1) % SS_COUNT);
            } else if (rotaryReadings < 0) {
                currentScreensaver = (ScreensaverType)((currentScreensaver - 1 + SS_COUNT) % SS_COUNT);
            }
            initActiveScreensaver();
        }
    }
    
    swRotary.update();
    if (swRotary.pressed()) {
        unsigned long now = millis();
        if (now - lastPressTime < 350) {
            // double click: skip song
            if (songInfo.format != "") {
                stopAudio = true;
                songInfo.paused = false;
                if(output) output->stop();
            }
        } else {
            // toggle play/pause
            if (songInfo.format != "") {
                if (songInfo.paused) {
                    handleResume();
                } else {
                    handlePause();
                }
            }
        }
        lastPressTime = now;
    }

    backBtn.update();
    if (backBtn.pressed() || backBtnLatched) {
        backBtnLatched = false;
        return true;
    }
    
    unsigned long now = millis();
    if (now - lastFrameTime >= 33) { // limit drawing to ~30 fps
        lastFrameTime = now;
        display.clearDisplay();
        
        switch (currentScreensaver) {
            case SS_DVD:
                updateAndDrawDvd();
                break;
            case SS_MATRIX:
                updateAndDrawMatrix();
                break;
            case SS_OSCILLOSCOPE:
                updateAndDrawOscilloscope();
                break;
            default:
                break;
        }
        
        drawScreensaverMusicOverlay();
        
        display.display();
    }
    return false;
}

