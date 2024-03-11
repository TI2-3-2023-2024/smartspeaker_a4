#include "recorder.h"

/**
 * @brief Static variables for audio recording module.
 * 
 * TAG: String identifier for logging purposes.
 * pipeline: Handle for the audio pipeline.
 * fatfs_stream_writer: Handle for writing audio data to FATFS stream.
 * i2s_stream_reader: Handle for reading audio data from I2S stream.
 * audio_encoder: Handle for encoding audio data.
 * evt: Handle for audio event interface.
 * set: Handle for ESP peripheral set.
 */
static const char *TAG = "RECORD_TO_SDCARD";
static audio_pipeline_handle_t pipeline;
static audio_element_handle_t fatfs_stream_writer, i2s_stream_reader, audio_encoder;
static audio_event_iface_handle_t evt;
static esp_periph_set_handle_t set;

void recorder_init(int sample_rate) {
    // Set logging levels
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Mount SD card and initialize peripheral set
    ESP_LOGI(TAG, "[ 1 ] Mount sdcard");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    set = esp_periph_set_init(&periph_cfg);

    // Initialize SD Card peripheral
    audio_board_sdcard_init(set, SD_MODE_1_LINE);

    // Start codec chip
    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_ENCODE, AUDIO_HAL_CTRL_START);

    // Create audio pipeline for recording
    ESP_LOGI(TAG, "[3.0] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    // Create FATFS stream to write data to SD card
    ESP_LOGI(TAG, "[3.1] Create fatfs stream to write data to sdcard");
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_WRITER;
    fatfs_stream_writer = fatfs_stream_init(&fatfs_cfg);

    // Create I2S stream to read audio data from codec chip
    ESP_LOGI(TAG, "[3.2] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_cfg.i2s_config.sample_rate = sample_rate;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

    // Create audio encoder to handle data
    ESP_LOGI(TAG, "[3.3] Create audio encoder to handle data");
    wav_encoder_cfg_t wav_cfg = DEFAULT_WAV_ENCODER_CONFIG();
    audio_encoder = wav_encoder_init(&wav_cfg);

    // Register all elements to audio pipeline
    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, audio_encoder, "wav");
    audio_pipeline_register(pipeline, fatfs_stream_writer, "file");

    // Link all elements together
    ESP_LOGI(TAG, "[3.5] Link it together [codec_chip]-->i2s_stream-->audio_encoder-->fatfs_stream-->[sdcard]");
    const char *link_tag[3] = {"i2s", "wav", "file"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    // Set music info to FATFS
    ESP_LOGI(TAG, "[3.6] Set music info to fatfs");
    audio_element_info_t music_info = {0};
    audio_element_getinfo(i2s_stream_reader, &music_info);
    ESP_LOGI(TAG, "[ * ] Save the recording info to the fatfs stream writer, sample_rates=%d, bits=%d, ch=%d",
                music_info.sample_rates, music_info.bits, music_info.channels);
    audio_element_setinfo(fatfs_stream_writer, &music_info);

    // Set up event listener for the audio pipeline
    ESP_LOGI(TAG, "[4] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    // Listen for events from the pipeline and peripherals
    ESP_LOGI(TAG, "[4.1] Listening event from pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);
}

void stop_record() {
    // Stop audio pipeline
    ESP_LOGI(TAG, "[8] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    // Unregister elements from the pipeline
    audio_pipeline_unregister(pipeline, audio_encoder);
    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, fatfs_stream_writer);

    // Terminate the pipeline and remove listener
    audio_pipeline_remove_listener(pipeline);

    // Stop all peripherals and remove event listener
    esp_periph_set_stop_all(set);
    audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);

    // Destroy event interface and release resources
    audio_event_iface_destroy(evt);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(fatfs_stream_writer);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(audio_encoder);
}


void create_recording(const char *string, int recording_length) {
    char filename[100];
    strcpy(filename, "/sdcard/");
    strcat(filename, string);

    ESP_LOGI(TAG, "[5] Set up  uri");
    audio_element_set_uri(fatfs_stream_writer, filename);
    
    ESP_LOGI(TAG, "[6] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[7] Listen for all pipeline events, record for %d Seconds", recording_length);
    int second_recorded = 0;
    while (1) {
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(evt, &msg, 1000 / portTICK_RATE_MS) != ESP_OK) {
            second_recorded++;
            ESP_LOGI(TAG, "[ * ] Recording ... %d", second_recorded);
            if (second_recorded >= recording_length) {
                audio_element_set_ringbuf_done(i2s_stream_reader);
            }
            continue;
        }
        // Stop when the last pipeline element (fatfs_stream_writer in this case) receives stop event
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) fatfs_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED)
                || ((int)msg.data == AEL_STATUS_ERROR_OPEN))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }
    stop_record();
}

void record_task(void *pvParameters) {
    // Initialize audio recording setup with a sample rate of 16000
    recorder_init(16000);

    // Create a recording named "eren.wav" with a duration of 5 seconds
    create_recording("eren.wav", 5);
    
    // Enter a loop to delay task execution
    while(1){
        vTaskDelay(10000/ portTICK_PERIOD_MS);
    }
}

void recorder_status(){

}

void start_speech_recognition() {
    
}




