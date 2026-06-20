Import("env")
import os

def apply_patch(path, old, new, label):
    if not os.path.exists(path):
        print(f"[patch] SKIP {label}: file not found at {path}")
        return
    with open(path, 'r') as f:
        content = f.read()
    if new in content:
        print(f"[patch] ALREADY APPLIED: {label}")
        return
    if old in content:
        content = content.replace(old, new)
        with open(path, 'w') as f:
            f.write(content)
        print(f"[patch] APPLIED: {label}")
    else:
        print(f"[patch] ERROR: {label} - block not found")

# --- Patch 1: sd_diskio.cpp - reduce sdSelectCard timeout from 500ms to 10ms ---
sd_diskio = os.path.join(
    env.subst("$PROJECT_PACKAGES_DIR"),
    "framework-arduinoespressif32",
    "libraries", "SD", "src", "sd_diskio.cpp"
)

apply_patch(
    sd_diskio,
    '  bool s = sdWait(pdrv, 500);',
    '  bool s = sdWait(pdrv, 10);',
    "sd_diskio sdSelectCard timeout 500ms -> 10ms"
)

apply_patch(
    sd_diskio,
    '      delay(100);\n      sdSelectCard(pdrv);\n      continue;\n    } else if (token & 0x08) {\n      log_w("crc error");\n      sdDeselectCard(pdrv);\n      delay(100);',
    '      delay(10);\n      sdSelectCard(pdrv);\n      continue;\n    } else if (token & 0x08) {\n      log_w("crc error");\n      sdDeselectCard(pdrv);\n      delay(10);',
    "sd_diskio sdCommand retry delay 100ms -> 10ms"
)

# --- Patch 2: AudioFileSourceSD.cpp - zero gain and set flags on read failure ---
audio_src = os.path.join(
    env.subst("$PROJECT_LIBDEPS_DIR"),
    env.subst("$PIOENV"),
    "ESP8266Audio", "src", "AudioFileSourceSD.cpp"
)

apply_patch(
    audio_src,
    '#include "AudioFileSourceSD.h"',
    '#include "AudioFileSourceSD.h"\n#include <AudioOutputI2S.h>\nextern volatile bool stopAudio;\nextern volatile bool sdCardRemoved;\nextern AudioOutputI2S* output;',
    "AudioFileSourceSD.cpp - add extern declarations"
)

apply_patch(
    audio_src,
    'uint32_t AudioFileSourceSD::read(void *data, uint32_t len) {\n    return f.read(reinterpret_cast<uint8_t*>(data), len);\n}',
    'uint32_t AudioFileSourceSD::read(void *data, uint32_t len) {\n    uint32_t bytesRead = f.read(reinterpret_cast<uint8_t*>(data), len);\n    if(bytesRead == 0 && len > 0) {\n        if(output) output->SetGain(0);\n        sdCardRemoved = true;\n        stopAudio = true;\n    }\n    return bytesRead;\n}',
    "AudioFileSourceSD.cpp - zero gain on read failure"
)

# --- Patch 3: AudioOutputI2S.cpp - return false from ConsumeSample if audioPaused ---
audio_out = os.path.join(
    env.subst("$PROJECT_LIBDEPS_DIR"),
    env.subst("$PIOENV"),
    "ESP8266Audio", "src", "AudioOutputI2S.cpp"
)

apply_patch(
    audio_out,
    'bool AudioOutputI2S::ConsumeSample(int16_t sample[2]) {',
    'bool AudioOutputI2S::ConsumeSample(int16_t sample[2]) {\n    extern volatile bool audioPaused;\n    if (audioPaused) {\n        return false;\n    }',
    "AudioOutputI2S.cpp - check audioPaused in ConsumeSample"
)

apply_patch(
    audio_out,
    '    chan_cfg.dma_frame_num = _bufferWords;\n    assert(ESP_OK == i2s_new_channel(&chan_cfg, &_tx_handle, nullptr));',
    '    chan_cfg.dma_frame_num = _bufferWords;\n    chan_cfg.auto_clear = true;\n    assert(ESP_OK == i2s_new_channel(&chan_cfg, &_tx_handle, nullptr));',
    "AudioOutputI2S.cpp - enable auto_clear in chan_cfg"
)
