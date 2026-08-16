# ESP32 YM2608 (OPNA) VGM Player

**Note:** This is an archived, unfinished project. I no longer have the hardware to test new changes, so this repository is kept as-is for portfolio and documentation purposes.

## Demo

[![VGM Player Demo](https://img.youtube.com/vi/W29TKDpeAnI/0.jpg)](https://www.youtube.com/watch?v=W29TKDpeAnI)

*(Click the image to watch a quick demo of the board running)*

## Overview

The goal of this project was to build a player that reads VGM data and drives a real YM2608 (OPNA) sound chip. The basic workflow is:

* Reads `.vgm` files directly from an SD card.
* Parses and validates the instructions.
* Sends the interpreted data to the YM2608 with the correct timing.
* Simple menu system to navigate the SD card and select files.

## Hardware

* Microcontroller: **ESP32 (ESP-WROOM-32 DevKit V1)**
* Sound Chip: YM2608 (OPNA)
* Storage: MicroSD Card Module

## Current State

**Working:**
* SD card reading and UI menu navigation.
* Basic `.vgm` parsing.
* FM channels playback.

**Known Issues / Missing Features:**
* Rhythm/percussion samples have playback glitches.
* ADPCM channel (external sample playback) is not implemented.

## Credits & Inspiration

This project was heavily inspired by [Fujix1's NanoDrive-Dual-OPN](https://github.com/Fujix1/NanoDrive-Dual-OPN). I highly recommend checking out their repository for schematics and a more complete implementation.

## License

This project is licensed under the terms of the **MIT License**. See the [LICENSE](LICENSE) file for details.
