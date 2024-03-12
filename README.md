# Recorder/Microphone Module

The Recorder/Microphone module enables audio recording capabilities using the ESP32_LyraT development board. It allows users to record audio and store it on an SD card connected to the board.

## Features

- **Audio Recording:** Record audio with specified parameters such as sample rate and duration.
- **Storage:** Save recorded audio files on an SD card.
- **Task Management:** Automate recording tasks using provided task functions.

## Getting Started

To get started with the Recorder/Microphone module, follow these steps:

1. **Installation:** Clone this repository to your local environment.
2. **Setup:** Ensure you have the ESP32_LyraT development board and an SD card connected.
3. **Build:** Compile the project using the provided build scripts or tools.
4. **Flash:** Flash the compiled firmware onto your ESP32_LyraT board.
5. **Usage:** Utilize the provided functions to initiate and manage audio recordings.

## Usage

The module provides several functions for audio recording:

- `recorder_init(sample_rate)`: Initialize the audio recording setup with the specified sample rate.
- `create_recording(filename, duration)`: Create a recording with the given filename and duration.
- `record_task()`: Task function to automate the recording process.

## Authors

Eren Zengin
