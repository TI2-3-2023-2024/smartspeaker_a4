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
extern const char *RECORDER_TAG;
extern audio_pipeline_handle_t pipeline;
extern audio_element_handle_t fatfs_stream_writer, i2s_stream_reader, audio_encoder;
extern audio_event_iface_handle_t evt;
extern esp_periph_set_handle_t set;
extern audio_board_handle_t board_handle;

void stop_record() {
    // Stop audio pipeline
    ESP_LOGI(RECORDER_TAG, "[8] Stop audio_pipeline");
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
    printf("record stopped\n");
}


void create_recording(const char *string, int recording_length) {
    char filename[100];
    strcpy(filename, "/sdcard/");
    strcat(filename, string);

    ESP_LOGI(RECORDER_TAG, "[5] Set up  uri");
    audio_element_set_uri(fatfs_stream_writer, filename);
    
    ESP_LOGI(RECORDER_TAG, "[6] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(RECORDER_TAG, "[7] Listen for all pipeline events, record for %d Seconds", recording_length);
    int second_recorded = 0;
    while (1) {
        printf("in while\n");
        audio_event_iface_msg_t msg;
        if (audio_event_iface_listen(evt, &msg, 1000 / portTICK_RATE_MS) != ESP_OK) {
            second_recorded++;
            printf("in OUTER if\n");
            ESP_LOGI(RECORDER_TAG, "[ * ] Recording ... %d", second_recorded);
            if (second_recorded >= recording_length) {
                printf("in inner if\n");
                audio_element_set_ringbuf_done(i2s_stream_reader);
            
            }
            printf("outside of if\n");
            continue;
        }
        // Stop when the last pipeline element (fatfs_stream_writer in this case) receives stop event
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) fatfs_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED)
                || ((int)msg.data == AEL_STATUS_ERROR_OPEN))) {
            ESP_LOGW(RECORDER_TAG, "[ * ] Stop event received");
            break;
        }
    }
    stop_record();
}

void recorder_status(){

}

void start_speech_recognition() {
    
}




