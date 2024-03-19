#include "init.h"
#include "sdcard_player.h"

extern audio_pipeline_handle_t pipeline;
extern audio_element_handle_t i2s_stream_writer, wav_decoder, fatfs_stream_reader, rsp_handle;
extern audio_event_iface_handle_t evt;
extern esp_periph_set_handle_t set;

const char* SDCARD_PLAYER = "SDCARD_PLAYER";

void sdcard_playlist(const char** filesArray, int arraySize)
{
    int currentFile = 0;
    play_next_file(filesArray[0]);
    while (1)
    {
        /* Handle event interface messages from pipeline
           to set music info and to advance to the next song
        */
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK)
        {
            ESP_LOGE(SDCARD_PLAYER, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT)
        {
            // Set music info for a new song to be played
            if (msg.source == (void *)wav_decoder && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)
            {
                audio_element_info_t music_info = {0};
                audio_element_getinfo(wav_decoder, &music_info);
                ESP_LOGW(SDCARD_PLAYER, "[ * ] Received music info from wav decoder, sample_rates=%d, bits=%d, ch=%d",
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
                        currentFile++;
                        if(currentFile >= arraySize){
                            break;
                        }

                        ESP_LOGW(SDCARD_PLAYER, "[ * ] Finished, advancing to the next file");
                        ESP_LOGW(SDCARD_PLAYER, "URL: %s", filesArray[currentFile]);
                        play_next_file(filesArray[currentFile]);
                }
                continue;
            }
        }
    }
    sdcard_player_stop();
}

void play_next_file(const char *sound_file) {
    // Stop any currently playing sound
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    
    // Set the URI to the sound file on the SD card
    char uri[32];
    snprintf(uri, 32, "/sdcard/NL/%s", sound_file);
    audio_element_set_uri(fatfs_stream_reader, uri);
    
    //Reset the pipeline and run it to play the sound
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
}


void sdcard_player_stop()
{
    ESP_LOGW(SDCARD_PLAYER, "[ 7 ] Stop audio_pipeline");
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
    //sdcard_list_destroy(sdcard_list_handle);
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(wav_decoder);
    audio_element_deinit(rsp_handle);
    //periph_service_destroy(input_ser);
    // esp_periph_set_destroy(set);
    printf("sdcard player stopped\n");
}