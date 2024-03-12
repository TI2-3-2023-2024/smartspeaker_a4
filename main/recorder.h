#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "board.h"
#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "wav_encoder.h"
#include "audio_idf_version.h"

#ifndef RECORDER_H
#define RECORDER_H

/* Define the standard record length for the recording in seconds */
#define RECORD_TIME_SECONDS (10)

/** 
 * @brief Facilitates audio recording and storage using ESP32_LyraT board.
 * 
 * This module enables audio recording and storage on an SD card connected 
 * to the ESP32_LyraT development board. It abstracts hardware interactions 
 * for simple recording management.
 */

//Function Declarations:

/**
 * @brief Sets up configurations for audio recording.
 * 
 * This function initializes the necessary configurations for the audio recording process.
 * It configures the audio module for recording with the specified sample rate.
 * 
 * @param sample_rate The sample rate for audio recording.
 */
void recorder_init(int sample_rate);

/**
 * @brief Stops the audio recording process.
 * 
 * This function stops the recording of audio data for speech recognition.
 * It finalizes the recording process and prepares the system to cease audio capture.
 */
void stop_record();

/**
 * @brief Creates a recording with the specified name and duration.
 * 
 * This function initiates the process of audio recording with the given name and duration.
 * The recorded audio will be saved under the provided name for the specified duration.
 * 
 * @param string The name of the recording.
 * @param recording_length The duration of the recording in seconds.
 */
void create_recording(const char *string, int recording_length);

/**
 * @brief Task function for recording audio.
 * 
 * This function is responsible for recording audio. It should be executed 
 * in a FreeRTOS task. The function manages the recording process, including
 * setup, duration, and termination.
 * 
 * @param pvParameters Pointer to task parameters (not used in this function).
 */
void record_task(void *pvParameters);


// Functie for checking the recorder status. // TODO;
void recorder_status();
// Function to start recording audio for speech recognition // TODO;
void start_speech_recognition();

#endif // RECORDER_H

// TODO: Functie voor het afspelen van opgenomen audio!!!!!
