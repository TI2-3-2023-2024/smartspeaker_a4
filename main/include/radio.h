#pragma once

// Include necessary libraries
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"

#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"
#include "custom_wifi.h"

#include "esp_netif.h"

// Define radio stream URLs
static const char* radio_streams[3] = {
    "https://www.mp3streams.nl/zender/radio-538/stream/4-mp3-128", 
    "https://www.mp3streams.nl/zender/qmusic-non-stop/stream/125-mp3-96", 
    "https://www.mp3streams.nl/zender/concertzender-klassiek/stream/110-mp3-128"};

/**
 * @brief Initializes the radio streaming functionality.
 *
 * This function sets up the necessary components for streaming radio content.
 * It prepares the audio pipeline, configures the Wi-Fi connection, and sets up
 * the audio output to the I2S interface.
 *
 * @param arg Optional argument for passing additional initialization parameters.
 */
void init_radio(void* is_running);
void stop_radio();
