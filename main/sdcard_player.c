/* Control with a touch pad playing WAV files from SD Card

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdcard_player.h"
#include "init.h"

extern const char *SDCARD_TAG;
extern audio_pipeline_handle_t pipeline;
extern audio_element_handle_t i2s_stream_writer, wav_decoder, fatfs_stream_reader, rsp_handle;
extern playlist_operator_handle_t sdcard_list_handle;
extern esp_periph_set_handle_t set;
extern audio_event_iface_handle_t evt;
extern char *url;
extern int player_volume;
extern audio_board_handle_t board_handle;
extern periph_service_handle_t input_ser;

void sdcard_player_start()
{
    while (1)
    {
        /* Handle event interface messages from pipeline
           to set music info and to advance to the next song
        */
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SDCARD_TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT)
        {
            // Set music info for a new song to be played
            if (msg.source == (void *)wav_decoder && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)
            {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(wav_decoder, &music_info);
                ESP_LOGW(SDCARD_TAG, "[ * ] Received music info from wav decoder, sample_rates=%d, bits=%d, ch=%d",
                         music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer, &music_info);
                rsp_filter_set_src_info(rsp_handle, music_info.sample_rates, music_info.channels);
                continue;
            }
            // Advance to the next song when previous finishes
            if (msg.source == (void *)i2s_stream_writer && msg.cmd == AEL_MSG_CMD_REPORT_STATUS)
            {
                audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
                if (el_state == AEL_STATE_FINISHED)
                {
                    ESP_LOGW(SDCARD_TAG, "[ * ] Finished, advancing to the next song");
                    // sdcard_player_stop();
                    sdcard_list_next(sdcard_list_handle, 1, &url);
                    ESP_LOGW(SDCARD_TAG, "URL: %s", url);
                    /* In previous versions, audio_pipeline_terminal() was called here. It will close all the element task and when we use
                     * the pipeline next time, all the tasks should be restarted again. It wastes too much time when we switch to another music.
                     * So we use another method to achieve this as below.
                     */
                    audio_element_set_uri(fatfs_stream_reader, url);
                    audio_pipeline_reset_ringbuffer(pipeline);
                    audio_pipeline_reset_elements(pipeline);
                    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
                    audio_pipeline_run(pipeline);
                }
                continue;
            }
        }
    }
}

void sdcard_player_stop()
{
    ESP_LOGW(SDCARD_TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, wav_decoder);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, rsp_handle);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Stop all peripherals before removing the listener */
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    sdcard_list_destroy(sdcard_list_handle);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(wav_decoder);
    audio_element_deinit(rsp_handle);
    periph_service_destroy(input_ser);
    esp_periph_set_destroy(set);
    printf("sdcard player stopped\n");
}

void sdcard_url_save_cb(void *user_data, char *url)
{
    sdcard_list_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_list_handle, url);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SDCARD_TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

// Callback function for input key service
esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    // Handle touch pad events to control music playback and adjust volume
    return handle_touchpad_event(evt, ctx);
}

// Handle touch pad events to control music playback and adjust volume
esp_err_t handle_touchpad_event(periph_service_event_t *evt, void *ctx)
{
    board_handle = (audio_board_handle_t)ctx;
    audio_hal_get_volume(board_handle->audio_hal, &player_volume);

    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE)
    {
        ESP_LOGW(SDCARD_TAG, "[ * ] input key id is %d", (int)evt->data);

        switch ((int)evt->data)
        {
        case INPUT_KEY_USER_ID_PLAY:
            ESP_LOGW(SDCARD_TAG, "[ * ] [Play] input key event");
            handle_play_pause_resume(audio_element_get_state(i2s_stream_writer));
            break;
        case INPUT_KEY_USER_ID_SET:
            ESP_LOGW(SDCARD_TAG, "[ * ] [Set] input key event");
            handle_next_song();
            break;
        case INPUT_KEY_USER_ID_VOLUP:
            ESP_LOGW(SDCARD_TAG, "[ * ] [Vol+] input key event");
            handle_volume_up();
            break;
        case INPUT_KEY_USER_ID_VOLDOWN:
            ESP_LOGW(SDCARD_TAG, "[ * ] [Vol-] input key event");
            handle_volume_down();
            break;
        case INPUT_KEY_USER_ID_MODE:
            ESP_LOGW(SDCARD_TAG, "[ * ] [MODE-] input key event");
            // handle_volume_down();
            break;
        case INPUT_KEY_USER_ID_REC:
            ESP_LOGW(SDCARD_TAG, "[ * ] [REC-] input key event");
            rec_handle();
            break;
        }
    }

    return ESP_OK;
}

hd44780_t lcd;
// extern bool codec_config;

void rec_handle()
{
    hd44780_clear(&lcd);
    write_string_on_pos(0, 0, "Recorder");
    write_char_on_pos(0, 1, 4);
    create_recording("eren.wav", 5);
}

// Play, pause, or resume music playback
void handle_play_pause_resume(audio_element_state_t el_state)
{
    switch (el_state)
    {
    case AEL_STATE_INIT:
        ESP_LOGW(SDCARD_TAG, "[ * ] Starting audio pipeline");
        audio_pipeline_run(pipeline);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGW(SDCARD_TAG, "[ * ] Pausing audio pipeline");
        audio_pipeline_pause(pipeline);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGW(SDCARD_TAG, "[ * ] Resuming audio pipeline");
        audio_pipeline_resume(pipeline);
        break;
    default:
        ESP_LOGW(SDCARD_TAG, "[ * ] Not supported state %d", el_state);
    }
}

// Advance to the next song
void handle_next_song()
{
    *url = NULL;
    ESP_LOGW(SDCARD_TAG, "[ * ] Stopped, advancing to the next song");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    sdcard_list_next(sdcard_list_handle, 1, &url);
    ESP_LOGW(SDCARD_TAG, "URL: %s", url);
    audio_element_set_uri(fatfs_stream_reader, url);
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_run(pipeline);
}

// Adjust volume up
void handle_volume_up()
{
    player_volume += 10;
    if (player_volume > 100)
    {
        player_volume = 100;
    }
    audio_hal_set_volume(board_handle->audio_hal, player_volume);
    ESP_LOGW(SDCARD_TAG, "[ * ] Volume set to %d %%", player_volume);
}

// Adjust volume down
void handle_volume_down()
{
    player_volume -= 10;
    if (player_volume < 0)
    {
        player_volume = 0;
    }
    audio_hal_set_volume(board_handle->audio_hal, player_volume);
    ESP_LOGW(SDCARD_TAG, "[ * ] Volume set to %d %%", player_volume);
}

void play_sound_by_filename(const char *file_name)
{
    // Concatenate ".wav" extension to the provided filename
    char full_file_name[16];
    snprintf(full_file_name, 16, "%s.wav", file_name);

    // Set the URI to the sound file on the SD card
    char uri[50];
    snprintf(uri, 50, "/sdcard/%s", full_file_name);
    audio_element_set_uri(fatfs_stream_reader, uri);

    // Reset the pipeline and run it to play the sound
    audio_pipeline_reset_ringbuffer(pipeline);

    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_run(pipeline);
    sdcard_player_stop();
}


