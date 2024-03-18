#pragma once

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "wav_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "input_key_service.h"
#include "periph_adc_button.h"
#include "board.h"

#include "sdcard_list.h"
#include "sdcard_scan.h"

void initialize_peripherals();
void setup_sdcard_playlist();
void start_codec_chip();
void create_input_key_service();
void create_audio_pipeline();
void create_audio_elements();
void set_up_event_listener();

void sdcard_player_init();
void sdcard_player_start();

void sdcard_url_save_cb(void *user_data, char *url);

esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);

esp_err_t handle_touchpad_event(periph_service_event_t *evt, void *ctx);
void handle_play_pause_resume(audio_element_state_t el_state);
void handle_next_song();
void handle_volume_up();
void handle_volume_down();
void play_sound(const char *sound_file);
void play_sound_by_filename(const char *sound_filename);


