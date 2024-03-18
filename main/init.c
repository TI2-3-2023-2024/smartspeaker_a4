#include "init.h"
#include "sdcard_player.h"
#include "lcd.h"

const char *RECORDER_TAG = "RECORD_TO_SDCARD";
audio_pipeline_handle_t pipeline;
audio_element_handle_t fatfs_stream_writer, i2s_stream_reader, audio_encoder;
audio_event_iface_handle_t evt;
esp_periph_set_handle_t set;
audio_board_handle_t board_handle;

const char *SDCARD_TAG = "SDCARD_PLAYER";

audio_element_handle_t i2s_stream_writer, wav_decoder, fatfs_stream_reader, rsp_handle;
playlist_operator_handle_t sdcard_list_handle = NULL;
char *url = NULL;
int player_volume = 80;
int sample_rate = 16000;
periph_service_handle_t input_ser;

const char *LCD_TAG = "LCD";
bool codec_config = false;
bool sdcard_mounted = false;
bool is_init_lcd = false;
bool is_init_inputservice = false;
bool is_sdcard_init = false;

typedef struct Element_Position
{
    int x;
    int y;
} Element_Position;

Element_Position element_position;
Element_Position page_position;

void app_init()
{
    ESP_ERROR_CHECK(i2cdev_init());

    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(RECORDER_TAG, ESP_LOG_INFO);

    if (!is_init_lcd)
    {    
        is_init_lcd = true;
        lcd_init();
    }

    initialize_peripherals();

    create_input_key_service();

    // Set up SD card playlist and scan music
    if(!is_sdcard_init){
        setup_sdcard_playlist();
        is_sdcard_init = true;
    }

    start_codec_chip();

    create_audio_pipeline();
}

/**
 * Static variable for PCF8574 I2C GPIO expander configuration.
 * The PCF8574 is used to interface with the LCD display, allowing for
 * data transmission over I2C, enabling the control of the LCD's digital inputs.
 */
i2c_dev_t pcf8574;

/**
 * Writes data to the LCD using the PCF8574 I2C GPIO expander.
 * @param lcd Pointer to the LCD configuration structure.
 * @param data Data to be written to the LCD.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

/**
 * Variable for HD44780 I2C LCD configuration.
 */
hd44780_t lcd;

void lcd_init()
{
    ESP_LOGW(LCD_TAG, "[1.0] LCD INIT STARTED");
    // LCD configuration
    lcd.write_cb = write_lcd_data; // use callback to send data to LCD by I2C GPIO expander
    lcd.font = HD44780_FONT_5X8;
    lcd.lines = 4;
    lcd.pins.rs = 0;
    lcd.pins.e = 2;
    lcd.pins.d4 = 4;
    lcd.pins.d5 = 5;
    lcd.pins.d6 = 6;
    lcd.pins.d7 = 7;
    lcd.pins.bl = 3;

    // Initialize PCF8574 I2C GPIO expander
    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, LCD_I2C_ADDRESS, 0, LCD_I2C_MASTER_SDA, LCD_I2C_MASTER_SCL));

    // Initialize LCD and enable backlight
    ESP_ERROR_CHECK(hd44780_init(&lcd));
    hd44780_switch_backlight(&lcd, true);
}

// Initialize peripherals management and audio board
void initialize_peripherals()
{
    ESP_LOGW(SDCARD_TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();

    set = esp_periph_set_init(&periph_cfg);

    ESP_LOGW(SDCARD_TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);

    if (!sdcard_mounted)
    {
        audio_board_sdcard_init(set, SD_MODE_1_LINE);
        sdcard_mounted = true;
    }
}

// Set up SD card playlist and scan music
void setup_sdcard_playlist()
{
    ESP_LOGW(SDCARD_TAG, "[1.2] Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_scan(sdcard_url_save_cb, "/sdcard", 0, (const char *[]){"wav"}, 1, sdcard_list_handle);
    sdcard_list_show(sdcard_list_handle);
}

// Start codec chip for audio playback
void start_codec_chip()
{
    ESP_LOGW(SDCARD_TAG, "[ 2 ] Start codec chip for both");
    board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    audio_hal_set_volume(board_handle->audio_hal, player_volume);
}

// Create and start input key service
void create_input_key_service()
{
    ESP_LOGW(SDCARD_TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, touchpad_handle, (void *)board_handle);
}

// Create audio pipeline for playback
void create_audio_pipeline()
{
    ESP_LOGW(SDCARD_TAG, "[4.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);
    
    //create_audio_elements();
}

// Create audio elements for the pipeline
void create_audio_elements()
{
    if (codec_config) // sdcard player init
    {
        ESP_LOGI(SDCARD_TAG, "[4.1] Create i2s stream to write data to codec chip");
        i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
        i2s_cfg.i2s_config.sample_rate = 48000;
        i2s_cfg.type = AUDIO_STREAM_WRITER;
        i2s_stream_writer = i2s_stream_init(&i2s_cfg);

        ESP_LOGI(SDCARD_TAG, "[4.4] Create fatfs stream to read data from sdcard");
        char *url = NULL;
        sdcard_list_current(sdcard_list_handle, &url);
        fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
        fatfs_cfg.type = AUDIO_STREAM_READER;
        fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
        audio_element_set_uri(fatfs_stream_reader, url);

        ESP_LOGW(SDCARD_TAG, "[4.2] Create wav decoder to decode wav file");
        wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
        wav_decoder = wav_decoder_init(&wav_cfg);

        ESP_LOGW(SDCARD_TAG, "[4.3] Create resample filter");
        rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
        rsp_cfg.src_rate = 16000;
        rsp_handle = rsp_filter_init(&rsp_cfg);

        ESP_LOGW(SDCARD_TAG, "[4.5] Register all elements to audio pipeline");
        audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
        audio_pipeline_register(pipeline, wav_decoder, "wav");
        audio_pipeline_register(pipeline, rsp_handle, "filter");
        audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

        ESP_LOGW(SDCARD_TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->wav_decoder-->resample-->i2s_stream-->[codec_chip]");
        const char *link_tag[4] = {"file", "wav", "filter", "i2s"};
        audio_pipeline_link(pipeline, &link_tag[0], 4);
        set_up_event_listener();
        ESP_LOGW(SDCARD_TAG, "SDCARD INIT FINISHED!");
    }
    else // recorder init
    {
        // Create FATFS stream to write data to SD card
        ESP_LOGI(RECORDER_TAG, "[3.1] Create fatfs stream to write data to sdcard");
        fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
        fatfs_cfg.type = AUDIO_STREAM_WRITER;
        fatfs_stream_writer = fatfs_stream_init(&fatfs_cfg);

        // Create I2S stream to read audio data from codec chip
        ESP_LOGI(RECORDER_TAG, "[3.2] Create i2s stream to read audio data from codec chip");
        i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
        i2s_cfg.type = AUDIO_STREAM_READER;
        i2s_cfg.i2s_config.sample_rate = sample_rate;
        i2s_stream_reader = i2s_stream_init(&i2s_cfg);

        // Create audio encoder to handle data
        ESP_LOGI(RECORDER_TAG, "[3.3] Create audio encoder to handle data");
        wav_encoder_cfg_t wav_cfg = DEFAULT_WAV_ENCODER_CONFIG();
        audio_encoder = wav_encoder_init(&wav_cfg);

        // Register all elements to audio pipeline
        ESP_LOGI(RECORDER_TAG, "[3.4] Register all elements to audio pipeline");
        audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
        audio_pipeline_register(pipeline, audio_encoder, "wav");
        audio_pipeline_register(pipeline, fatfs_stream_writer, "file");

        // Link all elements together
        ESP_LOGI(RECORDER_TAG, "[3.5] Link it together [codec_chip]-->i2s_stream-->audio_encoder-->fatfs_stream-->[sdcard]");
        const char *link_tag[3] = {"i2s", "wav", "file"};
        audio_pipeline_link(pipeline, &link_tag[0], 3);

        // Set music info to FATFS
        ESP_LOGI(RECORDER_TAG, "[3.6] Set music info to fatfs");
        audio_element_info_t music_info = {0};
        audio_element_getinfo(i2s_stream_reader, &music_info);
        ESP_LOGI(RECORDER_TAG, "[ * ] Save the recording info to the fatfs stream writer, sample_rates=%d, bits=%d, ch=%d",
                 music_info.sample_rates, music_info.bits, music_info.channels);
        audio_element_setinfo(fatfs_stream_writer, &music_info);

        // Set up event listener for the audio pipeline
        ESP_LOGI(RECORDER_TAG, "[4] Set up  event listener");
        audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
        evt = audio_event_iface_init(&evt_cfg);

        // Listen for events from the pipeline and peripherals
        ESP_LOGI(RECORDER_TAG, "[4.1] Listening event from pipeline");
        audio_pipeline_set_listener(pipeline, evt);

        ESP_LOGI(RECORDER_TAG, "[4.2] Listening event from peripherals");
        audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);
        ESP_LOGW(SDCARD_TAG, "RECORDER INIT FINISHED!");
    }
}

// Set up event listener for pipeline events
void set_up_event_listener()
{
    ESP_LOGW(SDCARD_TAG, "[5.0] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGW(SDCARD_TAG, "[5.1] Listen for all pipeline events");
    audio_pipeline_set_listener(pipeline, evt);
}
