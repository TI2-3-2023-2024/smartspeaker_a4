# smartspeaker_a4

## Overview LCD

This project implements a simple menu system for an LCD display using an ESP32_LyraT developement board. It allows for displaying 3 menu options on an LCD screen.

## Prerequisites
Before you begin, ensure you have met the following requirements:

- Visual Studio Code installed.
- ESP IDF framework installed.
- ESP IDF library installed from `https://github.com/UncleRus/esp-idf-lib`. This library is used to manipulate the LCD.
- An ESP32_LyraT development board.
- An 4 by 20 LCD screen.
- Some jumpwires.

## Installation

To install and run this project on your ESP32_LyraT, follow these steps:

1. Clone this repository to your local machine:

    ```bash
    git clone https://github.com/TI2-3-2023-2024/smartspeaker_a4.git
    ```

2. Open the project in the Visual Studio Code.

3. Modify the directory path specified in the line set(EXTRA_COMPONENT_DIRS C:/Projects/esp/external_libs/esp-idf-lib-v0.9.4/components) in the top-level CMakeLists.txt to correspond to the location where the library is installed on your computer.

4. Upload the code to your ESP32 board.

## Wiring

To be able to upload :

1. Connect the VCC, GND, SDA and SCL ports of your ESP32_LyraT board to the LCD display. You can use either 5V or 3.3V to power the LCD (this may vary depending on the specific LCD model).

2. Make sure that the LCD is powered on before uploading. Set the ESP32_LyraT to flash mode so that you can upload to the code to your board.

3. To set the ESP32_LyraT to flash mode, follow these steps:
    - Press and hold the boot button.
    - While still holding the boot button, press and release the reset button.
    - Finally, release the boot button. This action will set the ESP32_LyraT to flash mode.

5. Connect the ESP32_LyraT to your computer and run the code.

6. The menu system displays options such as "Internet Radio", "Sampler", and "Tuner" on the LCD screen.


## Acknowledgments

- This project uses the ESP-IDF framework.


