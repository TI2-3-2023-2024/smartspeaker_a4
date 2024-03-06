#pragma once

#include "esp_err.h"


void wifi_init();

void wifi_start();


/**
 * @brief Initialize the MP3 player with a URL
 * @param url The URL of the MP3 stream
 * @return ESP_OK on success, otherwise an error code
 */
void radio_player_init();

/**
 * @brief Start playback
 * @return ESP_OK on success, otherwise an error code
 */
void radio_player_start();

/**
 * @brief Stop playback
 * @return ESP_OK on success, otherwise an error code
 */
void radio_player_stop();

