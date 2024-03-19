#pragma once
#include "init.h"

void initialize_peripherals();
void setup_sdcard_playlist();
void start_codec_chip();
void create_input_key_service();
void create_audio_pipeline();
void create_audio_elements();
void set_up_event_listener();

void sdcard_player_init();
void sdcard_player_start();
void sdcard_player_stop();

void sdcard_url_save_cb(void *user_data, char *url);

esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);

esp_err_t handle_touchpad_event(periph_service_event_t *evt, void *ctx);
void handle_play_pause_resume(audio_element_state_t el_state);
void handle_next_song();
void handle_volume_up();
void handle_volume_down();
void play_sound(const char *sound_file);
void play_sound_by_filename(const char *sound_filename);


