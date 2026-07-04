#include <Arduino.h>
#include <ESP_I2S.h>
#include <BluetoothA2DPSinkQueued.h>

#include "bt.h"
#include "../globals/globals.h"
#include "../display/display.h"
#include "../helpers/helpers.h"
#include "../globals/settings.h"

I2SClass btI2S;
BluetoothA2DPSinkQueued a2dpSink(btI2S);

bool btModeActive = false;
bool btInitialized = false;
// keep the bluetooth device name in stable storage (avoid dangling pointers)
static String btName = "HC Soundbox";
volatile esp_a2d_connection_state_t btConnectionState = ESP_A2D_CONNECTION_STATE_DISCONNECTED;
volatile esp_a2d_audio_state_t btAudioState = ESP_A2D_AUDIO_STATE_STOPPED;
volatile bool btNeedsRedraw = true;
static volatile uint16_t btSampleRateHz = 44100;
static volatile uint16_t btRequestedSampleRateHz = 44100;
static volatile bool btSampleRateReinitPending = false;

static void onBtSampleRateChanged(uint16_t sample_rate) {
    // laptops commonly use 48khz while phones often use 44.1khz
    // if we keep i2s fixed at 44.1khz, 48khz audio will play slower/lower pitch and drift/skip
    if(sample_rate == 0) return;
    if(sample_rate == btSampleRateHz) return;

    // don't touch i2s inside the bt callback context; just request a re-init
    btRequestedSampleRateHz = sample_rate;
    btSampleRateReinitPending = true;
}

void onBtConnectionStateChanged(esp_a2d_connection_state_t state, void*) {
    btConnectionState = state;
    btNeedsRedraw = true;
}

void onBtAudioStateChanged(esp_a2d_audio_state_t state, void*) {
    btAudioState = state;
    btNeedsRedraw = true;
}

void onBtVolumeChanged(int vol) {
    // library reports volume as percent (0..100)
    volume = constrain(vol, 0, 100);
    btNeedsRedraw = true;
}

String connectionStateText() {
    if(btConnectionState == ESP_A2D_CONNECTION_STATE_CONNECTED) return "Connected";
    if(btConnectionState == ESP_A2D_CONNECTION_STATE_CONNECTING) return "Connecting";
    if(btConnectionState == ESP_A2D_CONNECTION_STATE_DISCONNECTING) return "Disconnecting";
    return "Disconnected";
}

void drawBluetoothScreen() {
    // snapshot volatile/shared state once for a consistent frame
    const esp_a2d_connection_state_t conn = btConnectionState;
    const int vol = constrain(volume, 0, 100);
    const bool isConnected = (conn == ESP_A2D_CONNECTION_STATE_CONNECTED);
    ::drawBluetoothScreen(isConnected, vol);
}

void startBluetoothMode(const char* deviceName) {
    if(btModeActive) return;

    if(deviceName && deviceName[0] != '\0') btName = deviceName;
    if(!btInitialized) {
        // guard against heap fragmentation crash in ringbuffer alloc
        size_t freeHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        if(freeHeap < 48 * 1024) {
            Serial.printf("[BT] Insufficient contiguous heap (%d bytes), restarting\n", freeHeap);
            ESP.restart();
        }
        btI2S.setPins(bclkPin, lrcPin, dinPin);
        btSampleRateHz = 44100;
        btRequestedSampleRateHz = btSampleRateHz;
        btSampleRateReinitPending = false;
        btI2S.begin(I2S_MODE_STD, btSampleRateHz, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH);

        // use queued output to decouple bt callback timing from i2s writes
        // laptops often push higher bitrate sbc; give the i2s writer more breathing room
        // keep ringbuffer small enough to always fit in fragmented heap
        // 96kb was causing intermittent alloc failures after mode switches
        a2dpSink.set_i2s_ringbuffer_size(32 * 1024);
        a2dpSink.set_i2s_ringbuffer_prefetch_percent(60);
        a2dpSink.set_i2s_write_size_upto(4096);
        a2dpSink.set_i2s_ticks(20);

        a2dpSink.set_on_connection_state_changed(onBtConnectionStateChanged);
        a2dpSink.set_on_audio_state_changed_post(onBtAudioStateChanged);
        a2dpSink.set_sample_rate_callback(onBtSampleRateChanged);
        a2dpSink.set_avrc_rn_volumechange(onBtVolumeChanged);
        a2dpSink.set_auto_reconnect(false);
        a2dpSink.set_volume(constrain(map(volume, 0, 100, 0, 127), 0, 127));
        a2dpSink.start(btName.c_str(), false);
        btInitialized = true;
    } else {
        a2dpSink.start(btName.c_str(), false);
    }

    btConnectionState = ESP_A2D_CONNECTION_STATE_CONNECTING;
    btAudioState = ESP_A2D_AUDIO_STATE_STOPPED;
    btNeedsRedraw = true;
    btModeActive = true;
}

bool handleBluetoothMode() {
    if(!btModeActive) return false;

    if(btSampleRateReinitPending) {
        const uint16_t target = btRequestedSampleRateHz;
        btSampleRateReinitPending = false;
        if(target != 0 && target != btSampleRateHz) {
            btSampleRateHz = target;
            btI2S.end();
            btI2S.setPins(bclkPin, lrcPin, dinPin);
            btI2S.begin(I2S_MODE_STD, btSampleRateHz, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH);
        }
    }

    int rotaryReadings = readRotary();
    if(rotaryReadings != 0) {
        if(rotaryReadings > 0 && volume < 100) volume = min(100, volume + rotaryReadings * Settings::volumeStep);
        else if(rotaryReadings < 0 && volume > 0) volume = max(0, volume + rotaryReadings * Settings::volumeStep);
 
        a2dpSink.set_volume(constrain(map(volume, 0, 100, 0, 127), 0, 127));
        btNeedsRedraw = true;
    }

    backBtn.update();
    if(backBtn.pressed()) {
        stopBluetoothMode();
        return true;
    }

    if(btNeedsRedraw) {
        // clear first so async callbacks during draw aren't lost
        btNeedsRedraw = false;
        drawBluetoothScreen();
    }

    delay(1);
    return false;
}

void stopBluetoothMode() {
    if(!btModeActive) return;
    a2dpSink.end(false);
    btConnectionState = ESP_A2D_CONNECTION_STATE_DISCONNECTED;
    btAudioState = ESP_A2D_AUDIO_STATE_STOPPED;
    btNeedsRedraw = true;
    btModeActive = false;
}

bool isBluetoothConnected() {
    return btConnectionState == ESP_A2D_CONNECTION_STATE_CONNECTED;
}
