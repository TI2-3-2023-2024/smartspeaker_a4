#pragma once
#include "esp_err.h"

static audio_element_handle_t create_i2s_stream(int sample_rate, audio_stream_type_t type);
static audio_element_handle_t create_resample_filter(int source_rate, int source_channels, int dest_rate, int dest_channels);
static audio_element_handle_t create_raw_stream();
static audio_pipeline_handle_t create_pipeline();


void frequency_detection_task(void* pvParameters);
esp_err_t init_goertzel_detector();
void start_goertzel_detection();
void stop_goertzel_detection();