#include "lcd.h"

/**
 * Static variable for PCF8574 I2C GPIO expander configuration.
 * The PCF8574 is used to interface with the LCD display, allowing for
 * data transmission over I2C, enabling the control of the LCD's digital inputs.
 */
static i2c_dev_t pcf8574;

static const char *TAG = "LCD";

typedef struct Element_Position
{
    int x;
    int y;
} Element_Position;

Element_Position element_position;
Element_Position page_position;

bool audio_player_mode;
extern audio_board_handle_t board_handle;
extern periph_service_handle_t input_ser;

/**
 * Bitmaps for LCD display icons, each icon consists of 8 rows to match LCD segment rows.
 * - tuner: Icon for tuner.
 * - internet_radio: Icon for internet radio.
 * - sampler: Icon for sampler.
 * - menu_pages: Icon for page structure for the menu.
 * - current_page: Icon for page structure that shows the current selected page.
 * - arrow: Arrow icon to show the current option in the menu.
 * - empty: Icon to help clear on certain areas.
 */
const uint8_t tuner[] = {0b00000, 0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b11111, 0b11111};
const uint8_t internet_radio[] = {0b00000, 0b00110, 0b01001, 0b10001, 0b10101, 0b10001, 0b01001, 0b00110};
const uint8_t sampler[] = {0b00000, 0b01000, 0b11000, 0b01000, 0b01110, 0b01111, 0b01110, 0b01100};
const uint8_t menu_pages[] = {0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111};
const uint8_t current_page[] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
const uint8_t arrow[] = {0b00000, 0b00100, 0b00010, 0b11111, 0b11111, 0b00010, 0b00100, 0b00000};
const uint8_t empty[] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};

/**
 * Writes data to the LCD using the PCF8574 I2C GPIO expander.
 * @param lcd Pointer to the LCD configuration structure.
 * @param data Data to be written to the LCD.
 * @return ESP_OK on success, or an error code on failure.
 */
static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

/**
 * Variable for HD44780 I2C LCD configuration.
 */
extern hd44780_t lcd;

void lcd_init()
{
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

    // Upload custom icons to LCD
    hd44780_upload_character(&lcd, 0, internet_radio);
    hd44780_upload_character(&lcd, 1, sampler);
    hd44780_upload_character(&lcd, 2, tuner);
    hd44780_upload_character(&lcd, 3, menu_pages);
    hd44780_upload_character(&lcd, 4, arrow);
    hd44780_upload_character(&lcd, 5, empty);
    hd44780_upload_character(&lcd, 6, current_page);

    create_input_service();
    page_position.x = 9;
    page_position.y = 0;
}

void create_input_service()
{
    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.1] Initialize and start peripherals");
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 3 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, menu_input_key_service, (void *)board_handle);
}

esp_err_t menu_input_key_service(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    return touchpad_menu_handle(evt, ctx);
}

/**
 * Initializes and displays a simple menu on the LCD.
 * @param pvParameters Pointer to task parameters (not used).
 */
void menu(void *pvParameters)
{
    lcd_init();
    // Writing the lines
    write_and_upload_char(1, 1, 0, " Internet Radio");
    write_and_upload_char(1, 2, 1, " Sampler");
    write_and_upload_char(1, 3, 2, " Tuner");

    for (int i = 0; i < 3; i++)
    {
        write_char_on_pos(i + 9, 0, 3);
    }
    write_char_on_pos(9, 0, 6);

    write_char_on_pos(0, 1, 4);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern bool audioboard_mode = false;

// Handle touch pad events to control music playback and adjust volume
esp_err_t touchpad_menu_handle(periph_service_event_t *evt, void *ctx)
{
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE)
    {
        ESP_LOGW(TAG, "[ * ] input key id is %d", (int)evt->data);

        switch ((int)evt->data)
        {
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGW(TAG, "[ * ] [Play] input key event");
                play_button_handle();
                break;
            case INPUT_KEY_USER_ID_SET:
                ESP_LOGW(TAG, "[ * ] [Set] input key event");
                set_button_handle();
                break;
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGW(TAG, "[ * ] [Vol+] input key event");
                vol_up_handle();
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGW(TAG, "[ * ] [Vol-] input key event");
                vol_down_handle();
                break;
            case INPUT_KEY_USER_ID_MODE:
                ESP_LOGW(TAG, "[ * ] [MODE-] input key event");
                mode_handle();
                break;
            case INPUT_KEY_USER_ID_REC:
                ESP_LOGW(TAG, "[ * ] [REC-] input key event");

                //rec_handle();
                break;
        }
    }

    return ESP_OK;
}

void play_button_handle()
{
    if (element_position.y != 3)
    {
        clear_at_position(element_position.x, element_position.y);
        element_position.y += 1;
        write_char_on_pos(element_position.x, element_position.y, 4);
    }
}

void set_button_handle()
{
    if (element_position.y != 1)
    {
        clear_at_position(element_position.x, element_position.y);
        element_position.y -= 1;
        write_char_on_pos(element_position.x, element_position.y, 4);
    }
}

void vol_up_handle()
{
    if (page_position.x != 11)
    {
        hd44780_clear(&lcd);
        write_char_on_pos(9, 0, 3);
        write_char_on_pos(10, 0, 3);
        write_char_on_pos(11, 0, 3);

        page_position.x += 1;
        write_char_on_pos(page_position.x, page_position.y, 6);
        if (page_position.x == 9)
        {
            write_and_upload_char(1, 1, 0, " Internet Radio");
            write_and_upload_char(1, 2, 1, " Sampler");
            write_and_upload_char(1, 3, 2, " Tuner");
        }
        else if (page_position.x == 10)
        {
            write_and_upload_char(1, 1, 0, " Recorder");
            write_and_upload_char(1, 2, 1, " Audio Speaker");
            write_and_upload_char(1, 3, 2, " Time");
        }
        else if (page_position.x == 11)
        {
            write_and_upload_char(1, 1, 0, " Eren");
            write_and_upload_char(1, 2, 1, " Matheus");
            write_and_upload_char(1, 3, 2, " Moustapha");
        }

        write_char_on_pos(0, 1, 4);
    }
}

void vol_down_handle()
{
    if (page_position.x != 9)
    {
        hd44780_clear(&lcd);
        write_char_on_pos(9, 0, 3);
        write_char_on_pos(10, 0, 3);
        write_char_on_pos(11, 0, 3);

        page_position.x -= 1;
        write_char_on_pos(page_position.x, page_position.y, 6);
        if (page_position.x == 9)
        {
            write_and_upload_char(1, 1, 0, " Internet Radio");
            write_and_upload_char(1, 2, 1, " Sampler");
            write_and_upload_char(1, 3, 2, " Tuner");
        }
        else if (page_position.x == 10)
        {
            write_and_upload_char(1, 1, 0, " Recorder");
            write_and_upload_char(1, 2, 1, " Audio Speaker");
            write_and_upload_char(1, 3, 2, " Time");
        }
        else if (page_position.x == 11)
        {
            write_and_upload_char(1, 1, 0, " Eren");
            write_and_upload_char(1, 2, 1, " Matheus");
            write_and_upload_char(1, 3, 2, " Moustapha");
        }

        write_char_on_pos(0, 1, 4);
    }
}

void mode_handle()
{
    // open content on specific page
    if (page_position.x == 9)
    {
        switch (element_position.y)
        {
        case 1:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Internet Radio");
            write_char_on_pos(0, 1, 4);

            break;
        case 2:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Sampler");
            write_char_on_pos(0, 1, 4);
            break;
        case 3:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Tuner");
            write_char_on_pos(0, 1, 4);
            break;
        }
    }
    else if (page_position.x == 10)
    {
        switch (element_position.y)
        {
        case 1:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Recorder");
            write_char_on_pos(0, 1, 4);
            rec_handle();
            break;
        case 2:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Audio Speaker");
            write_char_on_pos(0, 1, 4);
            break;
        case 3:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Time");
            write_char_on_pos(0, 1, 4);
            break;
        }
    }
    else if (page_position.x == 11)
    {
        switch (element_position.y)
        {
        case 1:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Eren");
            write_char_on_pos(0, 1, 4);
            break;
        case 2:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Matheus");
            write_char_on_pos(0, 1, 4);
            break;
        case 3:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Moustapha");
            write_char_on_pos(0, 1, 4);
            break;
        }
    }
}

void write_string_on_pos(int x, int y, const char *string)
{
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_puts(&lcd, string); // Output the specified string
}

void write_char_on_pos(int x, int y, char c)
{
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, c);      // Output the specified character
}

void write_and_upload_char(int x, int y, char c, const char *string)
{
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, c);      // Output the specified string
    hd44780_puts(&lcd, string); // Output the specified character
}

void clear_at_position(int x, int y)
{
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, 5);      // Output the specified string
}

void clear_line(int line)
{
    element_position.x = 0;
    element_position.y = line;
    if (line == 0 || line == 1 || line == 2 || line == 3)
    {
        hd44780_gotoxy(&lcd, 0, line);
        for (int i = 0; i < 20; i++)
        {
            clear_at_position(i, line);
        }
    }
    else
    {
        printf("Specified line to clean incorrect!\n");
    }
}