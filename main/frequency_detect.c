/**
 * Example ESP-ADF application using a Goertzel filter
 *
 * This application is an adaptation of the
 * Voice activity detection (VAD) example application in the
 * ESP-ADF framework by Espressif
 * https://github.com/espressif/esp-adf/tree/master/examples/speech_recognition/vad
 *
 * Goertzel algoritm initially implemented by P.S.M. Goossens,
 * adapted by Hans van der Linden
 *
 * Avans Hogeschool, Opleiding Technische Informatica
 */

#include <math.h>

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "filter_resample.h"

#include "goertzel_filter.h"
#include "frequency_detect.h"
#include "lcd.h"
#include "init.h"

#define LCD_UPDATE_RATE_MS 200

static const char *TAG = "FREQUENCY-DETECTOR";

bool timeout = false; // Timeout flag
bool running = true;

TimerHandle_t timeout_timer; // Timer for timeout
TimerHandle_t lcd_timer;

extern audio_pipeline_handle_t pipeline; // Audio pipeline handle

extern audio_element_handle_t i2s_stream_reader; // Audio element for I2S stream reader
extern audio_element_handle_t resample_filter;   // Audio element for resample filter
extern audio_element_handle_t raw_reader;        // Audio element for raw stream reader

extern goertzel_filter_cfg_t filters_cfg[GOERTZEL_NR_FREQS];   // Configuration for Goertzel filters
extern goertzel_filter_data_t filters_data[GOERTZEL_NR_FREQS]; // Data for Goertzel filters
extern int16_t *raw_buffer;                                    // Raw sample buffer

TaskHandle_t taskHandle;
QueueHandle_t xQueue;

VolumeMeter volumeMeter;

/**
 * @brief Create an I2S stream audio element.
 *
 * @param sample_rate Sample rate of the stream.
 * @param type Type of stream (reader or writer).
 * @return audio_element_handle_t Handle to the created audio element.
 */
audio_element_handle_t create_i2s_stream(int sample_rate, audio_stream_type_t type)
{
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = type;
    i2s_cfg.i2s_config.sample_rate = sample_rate;
    audio_element_handle_t i2s_stream = i2s_stream_init(&i2s_cfg);
    return i2s_stream;
}

/**
 * @brief Create a resample filter audio element.
 *
 * @param source_rate Source sample rate.
 * @param source_channels Number of source channels.
 * @param dest_rate Destination sample rate.
 * @param dest_channels Number of destination channels.
 * @return audio_element_handle_t Handle to the created audio element.
 */
audio_element_handle_t create_resample_filter(int source_rate, int source_channels, int dest_rate, int dest_channels)
{
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = source_rate;
    rsp_cfg.src_ch = source_channels;
    rsp_cfg.dest_rate = dest_rate;
    rsp_cfg.dest_ch = dest_channels;
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);
    return filter;
}

/**
 * @brief Create a raw stream audio element.
 *
 * @return audio_element_handle_t Handle to the created audio element.
 */
audio_element_handle_t create_raw_stream()
{
    raw_stream_cfg_t raw_cfg = {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    audio_element_handle_t raw_reader = raw_stream_init(&raw_cfg);
    return raw_reader;
}

/**
 * @brief Create an audio pipeline.
 *
 * @return audio_pipeline_handle_t Handle to the created audio pipeline.
 */
audio_pipeline_handle_t create_pipeline()
{
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audio_pipeline_handle_t pipeline = audio_pipeline_init(&pipeline_cfg);
    return pipeline;
}

/**
 * @brief Callback function for the LCD timer.
 *
 * @param xTimer Timer handle.
 */
void lcd_timer_callback(TimerHandle_t xTimer)
{
    lcd_timer = NULL;
}

/**
 * @brief Callback function for the timeout timer.
 *
 * @param xTimer Timer handle.
 */
void timeout_timer_callback(TimerHandle_t xTimer)
{
    ESP_LOGI(TAG, "Timeout passed");
    timeout = false;
}

/**
 * @brief Start the timeout timer.
 */
void start_timeout()
{
    ESP_LOGI(TAG, "Starting timeout timer");
    timeout = true;
    timeout_timer = xTimerCreate("TimeoutTimer", pdMS_TO_TICKS(TIMEOUT_MS), pdFALSE, (void *)0, timeout_timer_callback);
    xTimerStart(timeout_timer, pdMS_TO_TICKS(1000));
}

/**
 * @brief Detect frequency based on Goertzel filter magnitude.
 *
 * @param target_freq Target frequency to detect.
 * @param magnitude Magnitude calculated by the Goertzel filter.
 */
void detect_freq(int target_freq, float magnitude)
{
    float logMagnitude = 10.0f * log10f(magnitude);

    if (timeout)
        return;

    if (logMagnitude > GOERTZEL_DETECTION_THRESHOLD)
    {
        ESP_LOGI(TAG, "Detection at frequency %d Hz (magnitude %.2f, log magnitude %.2f)", target_freq, magnitude, logMagnitude);
        start_timeout();

        clear_line(1);

        char target[50];
        sprintf(target, "Detected freq: %d", target_freq);
        write_string_on_pos(0, 1, target);
    }
}

/**
 * @brief Initialize the Goertzel detector.
 *
 * @return esp_err_t ESP error code.
 */
esp_err_t init_goertzel_detector()
{
    running = true;
    volumeMeter.firstBar = 10;
    volumeMeter.secondBar = 10;
    // Setup audio codec
    ESP_LOGI(TAG, "Setup codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "Number of Goertzel detection filters is %d", GOERTZEL_NR_FREQS);

    ESP_LOGI(TAG, "Create raw sample buffer");
    raw_buffer = (int16_t *)malloc((GOERTZEL_BUFFER_LENGTH * sizeof(int16_t)));
    if (raw_buffer == NULL)
    {
        ESP_LOGE(TAG, "Memory allocation for raw sample buffer failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Setup Goertzel detection filters");
    for (int f = 0; f < GOERTZEL_NR_FREQS; f++)
    {
        filters_cfg[f].sample_rate = GOERTZEL_SAMPLE_RATE_HZ;
        filters_cfg[f].target_freq = GOERTZEL_DETECT_FREQS[f];
        filters_cfg[f].buffer_length = GOERTZEL_BUFFER_LENGTH;
        esp_err_t error = goertzel_filter_setup(&filters_data[f], &filters_cfg[f]);
        ESP_ERROR_CHECK(error);
    }

    ESP_LOGI(TAG, "Create pipeline");
    pipeline = create_pipeline();

    ESP_LOGI(TAG, "Create audio elements for pipeline");
    i2s_stream_reader = create_i2s_stream(AUDIO_SAMPLE_RATE, AUDIO_STREAM_READER);
    resample_filter = create_resample_filter(AUDIO_SAMPLE_RATE, 2, GOERTZEL_SAMPLE_RATE_HZ, 1);
    raw_reader = create_raw_stream();

    ESP_LOGI(TAG, "Register audio elements to pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, resample_filter, "rsp_filter");
    audio_pipeline_register(pipeline, raw_reader, "raw");

    ESP_LOGI(TAG, "Link audio elements together to make pipeline ready");
    const char *link_tag[3] = {"i2s", "rsp_filter", "raw"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "Start pipeline");
    audio_pipeline_run(pipeline);

    return ESP_OK;
}

void updateVolumeMeter(float magnitude)
{
    float logMagnitude = 10.0f * log10f(magnitude);
    updateVolume(&volumeMeter, logMagnitude * 2.5);
}

/**
 * @brief Start Goertzel frequency detection.
 */
void start_goertzel_detection()
{
    while (running)
    {
        if (raw_reader == NULL)
        {
            ESP_LOGE("A", "RAW_READER");
        }
        if (raw_buffer == NULL)
            ESP_LOGE("A", "RAW_BUFFER");

        // ESP_LOGE("A", "Reading raw stream");
        raw_stream_read(raw_reader, (char *)raw_buffer, GOERTZEL_BUFFER_LENGTH * sizeof(int16_t));
        // ESP_LOGE("A", "Read raw stream");

        int totalMagnitude = 0;
        for (int f = 0; f < GOERTZEL_NR_FREQS; f++)
        {
            float magnitude;

            if (!running) {
                break;
            }

            // ESP_LOGE("A", "Checking goertzelf filter process");
            esp_err_t error = goertzel_filter_process(&filters_data[f], raw_buffer, GOERTZEL_BUFFER_LENGTH);
            ESP_ERROR_CHECK(error);

            // ESP_LOGE("A", "Filter new magnitude");
            if (goertzel_filter_new_magnitude(&filters_data[f], &magnitude))
            {
                totalMagnitude = magnitude;
                detect_freq(filters_cfg[f].target_freq, magnitude);
            }
        }
        updateVolumeMeter((totalMagnitude / GOERTZEL_NR_FREQS));

        if (lcd_timer == NULL)
        {
            lcd_timer = xTimerCreate("LCD_Timer", pdMS_TO_TICKS(LCD_UPDATE_RATE_MS), pdFALSE, (void *)0, lcd_timer_callback);
            xTimerStart(lcd_timer, pdMS_TO_TICKS(1000));

            displayVolume(&volumeMeter, 2);
        }
    }

    ESP_LOGE(TAG, "EXITING");
    return;
}

/**
 * @brief Stop Goertzel frequency detection and clean up resources.
 */
void stop_goertzel_detection()
{
    if (!running)
        return;

    running = false;

    if (raw_buffer == NULL)
    {
        ESP_LOGE(TAG, "RAW_BUFFER IS NULL");
        return;
    }

    ESP_LOGE(TAG, "Deallocate raw sample buffer memory");
    free(raw_buffer);

    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, resample_filter);
    audio_pipeline_unregister(pipeline, raw_reader);

    audio_pipeline_deinit(pipeline);

    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(resample_filter);
    audio_element_deinit(raw_reader);
}

/**
 * @brief Task function for frequency detection.
 *
 * @param pvParameters Task parameters.
 */
void frequency_detection_task(void *pvParameters)
{
    // Set log levels
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // taskHandle = (TaskHandle_t) pvParameters;

    // Initialize Goertzel detector
    init_goertzel_detector();

    // Start Goertzel detection
    start_goertzel_detection();
}