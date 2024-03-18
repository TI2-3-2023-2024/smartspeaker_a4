/* Control with a touch pad playing WAV files from SD Card

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdcard_player.h"

static const char *TAG = "SDCARD_PLAYER";
audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_writer, wav_decoder, fatfs_stream_reader, rsp_handle;
playlist_operator_handle_t sdcard_list_handle = NULL;
esp_periph_set_handle_t set;
audio_event_iface_handle_t evt;
char *url = NULL;
int player_volume = 0;
audio_board_handle_t board_handle;
periph_service_handle_t input_ser;

void sdcard_player_init()
{
    initialize_peripherals();
    setup_sdcard_playlist();
    start_codec_chip();
    create_input_key_service();
    create_audio_pipeline();
    set_up_event_listener();

    ESP_LOGW(TAG, "[ 6 ] Press the keys to control music player:");
    ESP_LOGW(TAG, "      [Play] to start, pause and resume, [Set] next song.");
    ESP_LOGW(TAG, "      [Vol-] or [Vol+] to adjust volume.");
}

// Initialize peripherals management and audio board
void initialize_peripherals()
{
    ESP_LOGW(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();

    set = esp_periph_set_init(&periph_cfg);

    ESP_LOGW(TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);

    audio_board_sdcard_init(set, SD_MODE_1_LINE);
}

// Set up SD card playlist and scan music
void setup_sdcard_playlist()
{
    ESP_LOGW(TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_list_save(&sdcard_list_handle, "sdcard/");
    //sdcard_scan(sdcard_url_save_cb, "/sdcard", 0, (const char *[]){"wav"}, 1, sdcard_list_handle);
    sdcard_list_show(sdcard_list_handle);
}

// Start codec chip for audio playback
void start_codec_chip()
{
    ESP_LOGW(TAG, "[ 2 ] Start codec chip");
    board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
}

// Create and start input key service
void create_input_key_service()
{
    ESP_LOGW(TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, (void *)board_handle);
}

// Create audio pipeline for playback
void create_audio_pipeline()
{
    ESP_LOGW(TAG, "[4.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    create_audio_elements();
}

// Create audio elements for the pipeline
void create_audio_elements()
{
    ESP_LOGW(TAG, "[4.1] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    i2s_stream_set_clk(i2s_stream_writer, 48000, 16, 2);

    ESP_LOGW(TAG, "[4.2] Create wav decoder to decode wav file");
    wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
    wav_decoder = wav_decoder_init(&wav_cfg);

    ESP_LOGW(TAG, "[4.3] Create resample filter");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_handle = rsp_filter_init(&rsp_cfg);

    ESP_LOGW(TAG, "[4.4] Create fatfs stream to read data from sdcard");
    sdcard_list_current(sdcard_list_handle, &url);
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
    audio_element_set_uri(fatfs_stream_reader, url);

    ESP_LOGW(TAG, "[4.5] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, wav_decoder, "wav");
    audio_pipeline_register(pipeline, rsp_handle, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGW(TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->wav_decoder-->resample-->i2s_stream-->[codec_chip]");
    const char *link_tag[4] = {"file", "wav", "filter", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 4);
}

// Set up event listener for pipeline events
void set_up_event_listener()
{
    ESP_LOGW(TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGW(TAG, "[5.1] Listen for all pipeline events");
    audio_pipeline_set_listener(pipeline, evt);
}

void sdcard_player_start(){
    while (1)
    {
        /* Handle event interface messages from pipeline
           to set music info and to advance to the next song
        */
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT)
        {
            // Set music info for a new song to be played
            if (msg.source == (void *)wav_decoder && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)
            {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(wav_decoder, &music_info);
                ESP_LOGW(TAG, "[ * ] Received music info from wav decoder, sample_rates=%d, bits=%d, ch=%d",
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
                    ESP_LOGW(TAG, "[ * ] Finished, advancing to the next song");
                    // sdcard_player_stop();
                    sdcard_list_next(sdcard_list_handle, 1, &url);
                    ESP_LOGW(TAG, "URL: %s", url);
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

void sdcard_player_stop(){
    ESP_LOGW(TAG, "[ 7 ] Stop audio_pipeline");
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
}

void sdcard_url_save_cb(void *user_data, char *url)
{
    sdcard_list_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_list_handle, url);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

// Callback function for input key service
esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt,void *ctx)
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
        ESP_LOGW(TAG, "[ * ] input key id is %d", (int)evt->data);
        
        switch ((int)evt->data)
        {
        case INPUT_KEY_USER_ID_PLAY:
            ESP_LOGW(TAG, "[ * ] [Play] input key event");
            handle_play_pause_resume(audio_element_get_state(i2s_stream_writer));
            break;
        case INPUT_KEY_USER_ID_SET:
            ESP_LOGW(TAG, "[ * ] [Set] input key event");
            handle_next_song();
            break;
        case INPUT_KEY_USER_ID_VOLUP:
            ESP_LOGW(TAG, "[ * ] [Vol+] input key event");
            handle_volume_up();
            break;
        case INPUT_KEY_USER_ID_VOLDOWN:
            ESP_LOGW(TAG, "[ * ] [Vol-] input key event");
            handle_volume_down();
            break;
        case INPUT_KEY_USER_ID_MODE:
            ESP_LOGW(TAG, "[ * ] [MODE-] input key event");
            //handle_volume_down();
            break; 
        case INPUT_KEY_USER_ID_REC:
            ESP_LOGW(TAG, "[ * ] [REC-] input key event");
            // Handle maken
            break; 

        }
    }

    return ESP_OK;
}

// Play, pause, or resume music playback
void handle_play_pause_resume(audio_element_state_t el_state)
{
    switch (el_state)
    {
    case AEL_STATE_INIT:
        ESP_LOGW(TAG, "[ * ] Starting audio pipeline");
        audio_pipeline_run(pipeline);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGW(TAG, "[ * ] Pausing audio pipeline");
        audio_pipeline_pause(pipeline);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGW(TAG, "[ * ] Resuming audio pipeline");
        audio_pipeline_resume(pipeline);
        break;
    default:
        ESP_LOGW(TAG, "[ * ] Not supported state %d", el_state);
    }
}

// Advance to the next song
void handle_next_song()
{
    char *url = NULL;
    ESP_LOGW(TAG, "[ * ] Stopped, advancing to the next song");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    sdcard_list_next(sdcard_list_handle, 1, &url);
    ESP_LOGW(TAG, "URL: %s", url);
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
    ESP_LOGW(TAG, "[ * ] Volume set to %d %%", player_volume);
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
    ESP_LOGW(TAG, "[ * ] Volume set to %d %%", player_volume);
}

void play_sound(const char *sound_file) {
    // Stop any currently playing sound
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);
    
    // Set the URI to the sound file on the SD card
    char uri[15];
    snprintf(uri, 15, "/sdcard/NL%s", sound_file);
    audio_element_set_uri(fatfs_stream_reader, uri);
    
    // Reset the pipeline and run it to play the sound
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_run(pipeline);
}

void play_sound_by_filename(const char *sound_filename) {
    // Play sound directly by filename
    play_sound(sound_filename);
}


