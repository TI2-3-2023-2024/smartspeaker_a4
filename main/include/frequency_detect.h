#pragma once
#include "esp_err.h"

// Constants for Goertzel filter
#define GOERTZEL_SAMPLE_RATE_HZ 8000 // Sample rate in [Hz]
#define GOERTZEL_FRAME_LENGTH_MS 100 // Block length in [ms]
#define GOERTZEL_BUFFER_LENGTH (GOERTZEL_FRAME_LENGTH_MS * GOERTZEL_SAMPLE_RATE_HZ / 1000) // Buffer length in samples
#define GOERTZEL_DETECTION_THRESHOLD 47.0f // Detect a tone when log magnitude is above this value

// Audio capture settings
#define AUDIO_SAMPLE_RATE 48000 // Audio capture sample rate [Hz]
#define TIMEOUT_MS 100 // Timeout for detection in [ms]

// Frequencies to detect
static const int GOERTZEL_DETECT_FREQS[] = {
    2000,
    2250,
    2500,
    2750,
    3000,
    3250,
    3500,
    3750,
    4000};
#define GOERTZEL_NR_FREQS ((sizeof GOERTZEL_DETECT_FREQS) / (sizeof GOERTZEL_DETECT_FREQS[0])) // Number of frequencies to detect

static audio_element_handle_t create_i2s_stream(int sample_rate, audio_stream_type_t type);
static audio_element_handle_t create_resample_filter(int source_rate, int source_channels, int dest_rate, int dest_channels);
static audio_element_handle_t create_raw_stream();
static audio_pipeline_handle_t create_pipeline();


void frequency_detection_task(void* pvParameters);
esp_err_t init_goertzel_detector();
void start_goertzel_detection();
void stop_goertzel_detection();