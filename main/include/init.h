#pragma once

#include <stdio.h>
#include <string.h>
#include <ctype.h>
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

#include "wav_decoder.h"
#include "filter_resample.h"
#include "nvs_flash.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "input_key_service.h"
#include "periph_adc_button.h"

#include "mcp23x17.h"
#include <hd44780.h>
#include <pcf8574.h>

#include "sdcard_list.h"
#include "sdcard_scan.h"

void app_init();

void initialize_peripherals();
void setup_sdcard_playlist();
void start_codec_chip();
void create_input_key_service();
void create_audio_pipeline();
void create_audio_elements();
void create_alt_recording_elements();
void create_recording_elements();
void set_up_event_listener();

// void sdcard_player_stop();
// void stop_record();

/**
 * @brief Makes the necesseray configurations for the LCD and uploads custom characters.
 * 
 * This function is setting up the configurations for the LCD module and uploads the custom 
 * characters that are used for the menu. 
*/
void lcd_init();
esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data);