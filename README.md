<div align="center">

# 🎵 HC Soundbox

### A Portable MP3/WAV Player and Retro Console Built with ESP32

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-yellow.svg?style=for-the-badge)](https://opensource.org/license/gpl-3.0)
[![ESP32](https://img.shields.io/badge/ESP32-PlatformIO-E7352C?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=for-the-badge)](http://makeapullrequest.com)

**A standalone MP3/WAV player, retro game console, and ambient visualizer with OLED + rotary controls, supporting microSD playback, Bluetooth A2DP audio streaming, screensavers, games, and persistent system configurations — all powered by an ESP32.**

[Features](#-features) • [Quick Start](#-quick-start) • [Hardware](#-hardware) • [How to Use](#-how-to-use) • [Contributing](#-contributing)

</div>

---

## ✨ Features

<table>
<tr>
<td>

🎶 **MP3 & WAV Playback**  
Play music from microSD card via I2S DAC

📁 **File Browser**  
Navigate folders and files directly from the SD card with customizable sorting

🖥️ **OLED Display**  
0.96" screen with scrolling titles, menus, and progress bar

🎛️ **Rotary Encoder**  
Smooth navigation, menu control, and volume adjusting in one knob

🔊 **Real-Time Volume**  
Adjust gain on the fly with customizable volume step sizes

🌟 **Status RGB LED**  
Smart visual feedback dynamically matched to system mode, playback state, and games

</td>
<td>

📶 **Bluetooth Mode (A2DP Sink)**  
Stream audio from your phone over Bluetooth

📱 **Phone Volume Sync**  
Changing volume on your phone updates the ESP32 volume in real-time

👾 **Retro Games**  
Onboard rotary-controlled mini-games: Pong, Flappy Bird, and Breakout

🌌 **Screensavers**  
Classic animations: DVD Bouncing Logo, Matrix digital rain, and Oscilloscope

⚙️ **Persistent Settings**  
Adjust screen/LED brightness, volume defaults, and saving choices across reboots

⏭️ **Auto-Advance**  
Automatically plays the next track when a song ends

</td>
</tr>
</table>

---

## 🤔 Why HC Soundbox?

In an era of streaming services and smartphone apps, there's something special about a **physical music player you built yourself**.

### The Motivation
- 🎧 **Tangible music experience** — Physical controls beat touchscreens for music control
- 🛠️ **Learn by building** — Great project for understanding ESP32, I2S audio, displays, RGB PWM control, and FreeRTOS tasks
- 📵 **Distraction-free listening** — No notifications, no apps, just music and quick retro games
- 🎨 **Customizable** — Browse any folder structure, play any MP3 or WAV file, tweak screen brightness, and adjust sorting
- 💰 **Affordable** — Build a unique music player/retro mini-console for under $20

---

## 🚀 Quick Start

### Prerequisites

**Hardware:**
- ESP32 development board (e.g., NodeMCU-32S / ESP32 DevKitC)
- I2S DAC module (e.g. MAX98357A)
- 0.96" I2C OLED display (SSD1306)
- Rotary encoder with push button (EC11)
- 1 push button (back button)
- Common Cathode RGB LED
- Speaker (compatible with your DAC)
- microSD card module
- microSD card
- Jumper wires & breadboard

**Software:**
- [PlatformIO](https://platformio.org/) (VS Code extension recommended)

### Installation

**1. Clone the repository**
```bash
git clone https://github.com/HimC29/hc-soundbox.git
cd hc-soundbox
```

**2. Open in PlatformIO**

Open the project folder in VS Code with the PlatformIO extension installed. Dependencies are declared in [platformio.ini](file:///home/himc29/Projects/Projects/electronics/embedded/hc-soundbox/hc-soundbox/platformio.ini) and will be installed automatically:
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Bounce2`
- `ESP8266Audio`
- `ESP32-A2DP`

**3. (Optional) Prepare your microSD card**
- If you only want to use HC Soundbox as a **Bluetooth speaker/games console**, you can skip the microSD module + card entirely.
- Add songs into the card.
- No special naming convention required.
- Organize files however you like — the file browser mirrors your folder structure.
- Supported formats: `.mp3`, `.wav`.

**4. Wire the components**

See the [Hardware Connections](#-hardware-connections) section below.

**5. Upload the code**
- Connect your ESP32 via USB
- Click **Upload** in PlatformIO (ctrl + alt + u) ⬆️

**6. Insert SD card and enjoy! 🎉**

---

## 🔧 Hardware

### Components List

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Dev Board | 1 | Any standard ESP32 board |
| MAX98357A | 1 | I2S Mono Amp & DAC |
| OLED Display | 1 | 0.96" I2C (SSD1306) |
| Rotary Encoder | 1 | With push button (EC11 or similar) |
| Push Button | 1 | Momentary tactile switch (back) |
| Speaker | 1 | Compatible with your DAC |
| microSD Module | 1 | SPI interface |
| microSD Card | 1 | Any size (formatted as FAT32) |
| RGB LED | 1 | Common Cathode |

### 🔌 Hardware Connections

**You can view schematics instead in [HC_SoundBox.pdf](file:///home/himc29/Projects/Projects/electronics/embedded/hc-soundbox/hc-soundbox/schematics/HC_SoundBox.pdf)**

#### Rotary Encoder

| Encoder Pin | ESP32 Pin |
|-------------|-----------|
| CLK | GPIO 32 |
| DT | GPIO 33 |
| SW | GPIO 34 |
| VCC | 3.3V |
| GND | GND |

#### Back Button

| Button Pin | ESP32 Pin |
|------------|-----------|
| Signal | GPIO 4 |
| GND | GND |

#### MAX98357A

| Amp Pin | ESP32 Pin |
|---------|-----------|
| BCLK | GPIO 26 |
| LRC | GPIO 25 |
| DIN | GPIO 27 |
| VCC | 3.3V / 5V |
| GND | GND |

#### OLED Display (I2C)

| OLED Pin | ESP32 Pin |
|----------|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

#### microSD Module (SPI)

| SD Pin | ESP32 Pin |
|--------|-----------|
| CS | GPIO 5 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| SCK | GPIO 18 |
| VCC | 3.3V |
| GND | GND |

#### RGB LED

| LED Pin | ESP32 Pin |
|--------|-----------|
| Red | GPIO 14 |
| Green | GPIO 15 |
| Blue | GPIO 13 |

---

## 🎮 How to Use

### System Menu

On boot, you will be presented with the **System Menu**:
- **SD Card**: Browse and play audio files from microSD card
- **Bluetooth**: Stream audio from your phone/device via A2DP
- **Screensavers**: Run vintage graphics rendering modes
- **Games**: Play onboard mini-games using the rotary controller
- **Settings**: Tweak visual, audio, sorting, and system options

### SD Card Mode (How to play music)

1. Insert a **FAT32**-formatted microSD card containing `.mp3` / `.wav` files.
2. From the **System Menu**, select **SD Card**.
3. **Browse** using the rotary encoder:
   - Rotate = move selection up/down
   - Press = enter folder / play selected file
4. While playing:
   - Rotate = change volume
   - Press = pause / resume
   - Back = stop playback and return to the file browser
5. In the file browser:
   - Back = go up one directory (from the SD root, back returns to the main menu)

### Bluetooth Mode (How to connect)

1. Select **Bluetooth** in the System Menu.
2. Search for BT devices on your phone/tablet/computer and connect to **HC Soundbox**.
3. Play music or video audio on your device.
4. Back button disconnects/exits Bluetooth mode and returns to the System Menu.

### Screensaver Mode

1. Select **Screensavers** in the System Menu.
2. Use the rotary encoder to toggle between:
   - **DVD**: The retro bouncing logo bounce screen
   - **Matrix**: Dropping green digital code rain
   - **Oscilloscope**: Simulated waveform pattern generator
3. Press the back button to exit back to the System Menu.

### Games Mode

1. Select **Games** in the System Menu.
2. Choose from the three installed games:
   - **Pong**: Single-player vs an AI opponent. Rotate the encoder to move your paddle.
   - **Flappy Bird**: Press the rotary encoder switch to flap and guide the bird through obstacles.
   - **Breakout**: Destroy blocks by bouncing a ball with a paddle moved via the rotary encoder.
3. Press the back button to exit the current game or return to the System Menu.

### Settings Mode (Controls)

1. Select **Settings** in the System Menu.
2. Scroll to the setting you want to change:
   - **OLED Bright**: Adjust screen brightness (15 to 255)
   - **LED Bright**: Adjust RGB status LED brightness (0 to 255)
   - **Volume Step**: Volume increment/decrement step size (1 to 20)
   - **SD Def Vol**: Default volume value loaded when entering SD mode (0 to 100%)
   - **BT Def Vol**: Default volume value loaded when entering Bluetooth mode (0 to 100%)
   - **Song Sort**: Sort order for folder files (`A-Z`, `Z-A`, or standard `File Order`)
   - **Restore Defaults**: Overwrite custom configurations with factory defaults
3. Edit the selected configuration:
   - Press the encoder to enter edit mode (indicated by `<` and `>` wrapping the value).
   - Rotate the encoder to adjust the value.
   - Press the encoder again to save to NVS and exit edit mode.
   - Or, press the back button to cancel the change and restore the previous setting.

### Navigation Controls Summary

| Control | System / Menu Mode | SD Playback Mode | Settings (Edit Mode) | Game Mode |
|---------|--------------------|------------------|----------------------|-----------|
| **Rotate Encoder** | Scroll menu selection | Adjust volume | Adjust value | Move paddle / control |
| **Press Encoder** | Select item / Enter | Pause / Resume | Confirm and save | Game start / Flap (Flappy) |
| **Back Button** | Go back / Up folder | Stop playback | Cancel edit | Exit game / Exit to Menu |

### RGB Status LED States

| LED Behavior | Meaning |
|--------------|---------|
| **Solid White** | Menu navigation or system idle |
| **Cycling Rainbow** | Active music playback (SD Mode) |
| **Solid Purple** | Paused SD Card music playback |
| **Solid Blue** | Bluetooth connected |
| **Solid Red** | Bluetooth pairing mode (disconnected) |
| **Custom Colors & Flashes** | Game feedback cues (e.g. Purple in Pong, Blue in Flappy, Green in Breakout; flashing on score/loss events) |

---

## 📂 Music Organization

The file browser mirrors your SD card's folder structure exactly — no special naming required.

```
SD Card Root/
├── Rock/
│   ├── song1.mp3
│   └── song2.wav
├── Jazz/
│   └── track1.mp3
└── favourite.mp3
```

> ⚠️ **Note:** Only `.mp3` and `.wav` files are playable. Other file types are visible in the browser but cannot be selected for playback.

---

## 💻 Software Architecture

The project is structured as a PlatformIO C++ project with the following modules:

```
src/
├── main.cpp          — Setup, main loop, and global app mode state machine
├── bt/
│   ├── bt.cpp         — Bluetooth A2DP sink mode (connect/stream/volume sync)
│   └── bt.h
├── controls/
│   ├── controls.cpp   — Settings page rendering, item selection, and adjust handlers
│   └── controls.h
├── display/
│   ├── display.cpp    — SSD1306 OLED rendering (menus, playback screens, dialogs)
│   └── display.h
├── games/
│   ├── games.cpp      — Engines, rendering, and logic for Pong, Flappy Bird, and Breakout
│   └── games.h
├── globals/
│   ├── globals.cpp    — Shared hardware pin mapping, display instances, state flags
│   ├── globals.h
│   ├── settings.cpp   — EEPROM/Preferences wrapper to save/load settings to ESP32 Flash
│   └── settings.h
├── helpers/
│   ├── helpers.cpp    — Rotary encoder decoding and button helpers
│   └── helpers.h
├── rgb/
│   ├── rgb.cpp        — RGB LED color controls, gamma levels, and rainbow animations
│   └── rgb.h
├── screensaver/
│   ├── screensaver.cpp— Matrix digital rain, DVD logo bounce, and Oscilloscope routines
│   └── screensaver.h
├── sd/
│   ├── sdAudio.cpp    — SD audio playback pipeline (I2S decoding, task setups)
│   ├── sdAudio.h
│   ├── sdGlobals.cpp  — SD audio playback variables
│   ├── sdGlobals.h
│   ├── sdMenu.cpp     — SD file browser menu drawing and logic
│   ├── sdMenu.h
│   ├── sdState.cpp    — SD mode execution flow, sorting logic, and state machine
│   └── sdState.h
└── scrollText/
    ├── scrollText.cpp — Scrolling text animation helpers for long titles
    └── scrollText.h
```

### Key Implementation Details

- **FreeRTOS audio task** — Audio decoding runs on Core 0 via `xTaskCreatePinnedToCore`, keeping the UI rendering and button scanning responsive on Core 1.
- **Persistent Memory (NVS)** — Utilizes the ESP32 `Preferences` library to write settings (brightness, volumes, sorting) directly to non-volatile flash memory.
- **Custom Screen Refresh** — Screensavers and games bypass standard polling loops to perform high-frequency rendering routines directly.
- **Pause/Resume** — Saves the byte position in the file (`source->getPos()`) and seeks back on resume.
- **Song length** — Parsed directly from WAV headers (`fmt`/`data` chunks) and MP3 Xing/Info VBR headers, with a 128kbps CBR fallback.
- **Scrolling display** — Long filenames and directory paths scroll automatically on the OLED.
- **Bounce2 debouncing** — All buttons use hardware debouncing via the Bounce2 library.
- **Bluetooth A2DP sink** — Bluetooth audio is handled in `bt/` using `ESP32-A2DP` with queued I2S output.
- **Phone volume sync** — When the connected device changes volume, AVRCP volume change callbacks update the ESP32 `volume` value and refresh the UI.

### Libraries Used

- `ESP8266Audio` — I2S audio decoding for MP3 and WAV
- `Adafruit_GFX` + `Adafruit_SSD1306` — OLED display driver
- `Bounce2` — Button debouncing
- `ESP32-A2DP` — Bluetooth A2DP audio sink (streaming from phone)

---

## 🤝 Contributing

Contributions are what make the open-source community such an amazing place! Any contributions you make are **greatly appreciated**.

### How to Contribute

1. **Fork the Project**
2. **Create your Feature Branch**
   ```bash
   git checkout -b feature/AmazingFeature
   ```
3. **Commit your Changes**
   ```bash
   git commit -m 'Add some AmazingFeature'
   ```
4. **Push to the Branch**
   ```bash
   git push origin feature/AmazingFeature
   ```
5. **Open a Pull Request**

### Ideas for Contributions

- 🎨 Custom case designs (3D printable STL files)
- 🔋 Battery power support with charge level indicator
- 🔀 Shuffle and repeat modes
- 💾 Remember last played file across reboots
- 🌈 Expanded LED visualizer synced to audio frequency
- 📖 Multi-language filename support

---

## 🌟 Contributors

Thanks to everyone who has contributed to HC Soundbox!

<a href="https://github.com/HimC29/hc-soundbox/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=HimC29/hc-soundbox" />
</a>

Want to see your name here? Check out the [Contributing](#-contributing) section!

---

## 🐛 Troubleshooting

### No sound from speaker
- Check I2S DAC wiring (BCLK → 26, LRC → 25, DIN → 27)
- Verify the DAC module is powered correctly
- Confirm the file format is `.mp3` or `.wav`

### Bluetooth device not showing up / can’t pair
- Make sure you selected **Bluetooth** mode in the System Menu (it only advertises/acts as a speaker in BT mode)
- Remove/forget **HC Soundbox** from your phone’s Bluetooth list and try pairing again
- Reboot the ESP32 and retry

### Bluetooth connects but no audio
- Confirm your phone is actually outputting audio to **HC Soundbox** (check the audio output route)
- Try starting playback after the connection finishes
- If you were previously in SD mode, go back to the System Menu and re-enter Bluetooth mode

### Phone volume changes don’t update the ESP32 volume
- Some phones/players don’t send AVRCP absolute volume updates in all cases; try changing volume from the system volume buttons (not in-app)
- Reconnect Bluetooth and try again

### OLED display not working
- Verify I2C address is `0x3C` (run an I2C scanner sketch to confirm)
- If I2C address is not `0x3C`, change the value of DISPLAY_ADDRESS found near the end of [globals.cpp](file:///home/himc29/Projects/Projects/electronics/embedded/hc-soundbox/hc-soundbox/src/globals/globals.cpp)
- Check SDA/SCL connections to GPIO 21/22

### SD card not detected
- Ensure the card is formatted as **FAT32**
- Check SPI wiring (CS → GPIO 5)
- Try a different SD card or re-seat the card

### Settings are corrupted or behaving weirdly
- Enter **Settings** in the System Menu and choose **Restore Defaults**. This resets all configurations stored in the Preferences (NVS) memory block.

### Songs not advancing automatically
- Check Serial Monitor for error messages from the audio task
- Ensure the next file in the directory is a supported format

---

## 📄 License

Distributed under the GNU GPL v3 License. See [LICENSE](file:///home/himc29/Projects/Projects/electronics/embedded/hc-soundbox/hc-soundbox/LICENSE) for more information.

**TL;DR:** You can use, modify, and distribute this project freely. Just keep the original license notice and make your modified code open source.

---

## 🙏 Acknowledgments

Built with amazing open-source tools and libraries:

- **[Espressif / ESP32](https://www.espressif.com/)** — The platform that powers this project
- **[Adafruit](https://www.adafruit.com/)** — For excellent display libraries
- **[earlephilhower/ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)** — I2S audio decoding library
- **[thomasfredericks/Bounce2](https://github.com/thomasfredericks/Bounce2)** — Button debouncing library
- **[pschatzmann/ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP)** — Bluetooth A2DP audio streaming support
- **All contributors** who have helped improve this project

---

<div align="center">

### ⭐ Star this repo if you build one!

**Made with ❤️ by [HimC29](https://github.com/HimC29)**

[Report Bug](https://github.com/HimC29/hc-soundbox/issues) • [Request Feature](https://github.com/HimC29/hc-soundbox/issues)

</div>
