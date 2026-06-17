<div align="center">

# 🎵 HC Soundbox

### A Portable MP3/WAV Player Built with ESP32

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-yellow.svg?style=for-the-badge)](https://opensource.org/license/gpl-3.0)
[![ESP32](https://img.shields.io/badge/ESP32-PlatformIO-E7352C?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=for-the-badge)](http://makeapullrequest.com)

**A standalone MP3/WAV player with OLED + rotary controls, with 2 modes: microSD playback and Bluetooth A2DP audio — all powered by an ESP32.**

[Features](#-features) • [Quick Start](#-quick-start) • [Hardware](#-hardware) • [Contributing](#-contributing)

</div>

---

## ✨ Features

<table>
<tr>
<td>

🎶 **MP3 & WAV Playback**
Play music from microSD card via I2S DAC

📁 **File Browser**
Navigate folders and files directly from the SD card

🖥️ **OLED Display**
0.96" screen with scrolling titles and progress bar

🎛️ **Rotary Encoder**
Smooth navigation and volume control in one knob

🔊 **Real-Time Volume**
Adjust gain on the fly with the rotary encoder

</td>
<td>

📶 **Bluetooth Mode (A2DP Sink)**
Stream audio from your phone over Bluetooth

📱 **Phone Volume Sync**
Changing volume on your phone updates the ESP32 volume

⏭️ **Auto-Advance**
Automatically plays the next track when a song ends

⏸️ **Pause & Resume**
Resume from exactly where you left off

⚡ **USB Powered**
Portable with any USB power bank

</td>
</tr>
</table>

---

## 🤔 Why HC Soundbox?

In an era of streaming services and smartphone apps, there's something special about a **physical music player you built yourself**.

### The Motivation
- 🎧 **Tangible music experience** — Physical controls beat touchscreens for music control
- 🛠️ **Learn by building** — Great project for understanding ESP32, I2S audio, displays, and FreeRTOS tasks
- 📵 **Distraction-free listening** — No notifications, no apps, just music
- 🎨 **Customizable** — Browse any folder structure, play any MP3 or WAV file
- 💰 **Affordable** — Build a unique music player for under $20

---

## 🚀 Quick Start

### Prerequisites

**Hardware:**
- ESP32 development board
- I2S DAC module (e.g. MAX98357A)
- 0.96" I2C OLED display (SSD1306)
- Rotary encoder with push button
- 1 push button (back)
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

Open the project folder in VS Code with the PlatformIO extension installed. Dependencies are declared in `platformio.ini` and will be installed automatically:
- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `Bounce2`
- `ESP8266Audio`
- `ESP32-A2DP`

**3. (Optional) Prepare your microSD card**
- If you only want to use HC Soundbox as a **Bluetooth speaker**, you can skip the microSD module + card entirely.
- Add songs into the card
- No special naming convention required
- Organize files however you like — the file browser mirrors your folder structure
- Supported formats: .mp3, .wav

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
| MAX98357A | 1 |  |
| OLED Display | 1 | 0.96" I2C (SSD1306) |
| Rotary Encoder | 1 | With push button (EC11 or similar) |
| Push Button | 1 | Momentary tactile switch (back) |
| Speaker | 1 | Compatible with your DAC |
| microSD Module | 1 | SPI interface |
| microSD Card | 1 | Any size |
| RGB LED | 1 |  |

### 🔌 Hardware Connections

**You can view schematics instead in [HC_SoundBox.pdf](schematics/HC_SoundBox.pdf)**

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

On boot you’ll see a **System Menu**:
- **SD Card**: browse and play files from microSD
- **Bluetooth**: stream audio from your phone (A2DP)

### SD Card Mode (How to play music)

1. Insert a **FAT32**-formatted microSD card with `.mp3` / `.wav` files.
2. From **System Menu**, choose **SD Card**.
3. **Browse** with the rotary encoder:
   - Rotate = move selection up/down
   - Press = enter a folder / play a file
4. While playing:
   - Rotate = change volume
   - Press = pause / resume
   - Back = stop playback and return to the file browser
5. In the file browser:
   - Back = go up one directory (and from SD root, back returns to mode select)

### Bluetooth Mode (How to connect)

1. Select **Bluetooth** in the System Menu.
2. Search for BT devices on your phone/tablet/computer and connect to **HC Soundbox**.
3. Play music or videos on your device.
4. Back button returns you to the System Menu.

### Navigation Controls

| Control | Action |
|---------|--------|
| **Rotate encoder** | Scroll up/down in menus / Adjust volume (SD + Bluetooth) |
| **Press encoder** | Select menu item / Enter folder / Play selected file / Pause & Resume (SD) |
| **Back button** | Go back (SD: up directory / stop playback, BT: System Menu) |

### Playback Screen

While a song is playing, the display shows:
- Scrolling song title
- Current position / total length progress bar
- Current volume level
- Pause/Resume button indicator

### Auto-Advance

When a song finishes, the player automatically plays the next file in the current directory. It wraps back to the first file after the last one.

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
├── main.cpp          — Setup, loop, and page state machine
├── bt/
│   ├── bt.cpp         — Bluetooth A2DP sink mode (connect/stream/volume sync)
│   └── bt.h
├── display/
│   ├── display.cpp    — OLED rendering (menus, SD screens, Bluetooth screen)
│   └── display.h
├── globals/
│   ├── globals.cpp    — Shared state definitions (pins, display, volume, etc.)
│   └── globals.h      — Shared state declarations
├── helpers/
│   ├── helpers.cpp    — Rotary + small helpers
│   └── helpers.h
├── sd/
│   ├── sdAudio.cpp    — SD audio playback plumbing
│   ├── sdAudio.h
│   ├── sdGlobals.cpp  — SD-related shared state
│   ├── sdGlobals.h
│   ├── sdMenu.cpp     — SD file browser UI logic
│   ├── sdMenu.h
│   ├── sdState.cpp    — SD mode state machine
│   └── sdState.h
└── scrollText/
    ├── scrollText.cpp — Scrolling text animation helpers
    └── scrollText.h
```

### Key Implementation Details

- **FreeRTOS audio task** — Audio decoding runs on Core 0 via `xTaskCreatePinnedToCore`, keeping the UI responsive on Core 1
- **Pause/Resume** — Saves the byte position in the file (`source->getPos()`) and seeks back on resume
- **Song length** — Parsed directly from WAV headers (`fmt`/`data` chunks) and MP3 Xing/Info VBR headers, with a 128kbps CBR fallback
- **Scrolling display** — Long filenames and directory paths scroll automatically on the OLED
- **Bounce2 debouncing** — All buttons use hardware debouncing via the Bounce2 library
- **Bluetooth A2DP sink** — Bluetooth audio is handled in `bt/` using `ESP32-A2DP` with queued I2S output
- **Phone volume sync** — When the connected device changes volume, AVRCP volume change callbacks update the ESP32 `volume` value and refresh the UI

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
- 🌈 LED visualizer synced to audio
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
- If I2C address is not `0x3C`, change the value of DISPLAY_ADDRESS found near the end of [globals.cpp](src/globals/globals.cpp)
- Check SDA/SCL connections to GPIO 21/22

### SD card not detected
- Ensure the card is formatted as **FAT32**
- Check SPI wiring (CS → GPIO 5)
- Try a different SD card or re-seat the card

### Songs not advancing automatically
- Check Serial Monitor for error messages from the audio task
- Ensure the next file in the directory is a supported format

---

## 📄 License

Distributed under the GNU GPL v3 License. See [LICENSE](LICENSE) for more information.

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
