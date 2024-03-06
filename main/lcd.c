#include "lcd.h"
#include "string.h"

static i2c_dev_t pcf8574;

// Assigning an alias to SDA en SCL on the ESP_LyraT board are these SDA: 18 and SCL: 23
// on the ESP-WROOM32 these are SDA: 33 and SCL: 32
#define CONFIG_EXAMPLE_I2C_MASTER_SDA 18
#define CONFIG_EXAMPLE_I2C_MASTER_SCL 23
#define CONFIG_EXAMPLE_I2C_ADDR 0x27

// Charsets for creating icons
const uint8_t tuner[] = {0b00000, 0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b11111, 0b11111};
const uint8_t internet_radio[] = {0b00000, 0b00110, 0b01001, 0b10001, 0b10101, 0b10001, 0b01001, 0b00110};
const uint8_t sampler[] = {0b00000, 0b00100, 0b01010, 0b10101, 0b10001, 0b10001, 0b10001, 0b01110};

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

// Initializing a simple menu
void menu(void *pvParameters)
{
    // LCD properties
    hd44780_t lcd = {
        .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
        .font = HD44780_FONT_5X8,
        .lines = 4,
        .pins = {
            .rs = 0,
            .e = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3}};

    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, CONFIG_EXAMPLE_I2C_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL));

    ESP_ERROR_CHECK(hd44780_init(&lcd));

    hd44780_switch_backlight(&lcd, true);

    // Uploading the charsets so they can be used the 0,1,2 are the numbers assigned to them to call the specific charset
//    hd44780_upload_character(&lcd, 0, internet_radio);
//    hd44780_upload_character(&lcd, 1, sampler);
//    hd44780_upload_character(&lcd, 2, tuner);

    // Clear LCD
    hd44780_clear(&lcd);

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[6];

    strftime(strftime_buf, sizeof(strftime_buf), "%H:%M", &timeinfo);

    // Writing something
    write_string_on_pos(&lcd, 8, 1, strftime_buf);
    ESP_LOGI("LCD PRINT", "print to screen");


    while (1)
    {
        ESP_LOGI("TIMESYNC: ", "The time is: %s ", strftime_buf);
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

        write_string_on_pos(&lcd, 7, 1, strftime_buf);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Writes a string on the given postion
void write_string_on_pos(const hd44780_t *lcd, int x, int y, const char *string)
{
    hd44780_gotoxy(lcd, x, y); // Setting cursor to designated area
    hd44780_puts(lcd, string); // Writing the string
}

// Writes a char on the given postion
void write_char_on_pos(const hd44780_t *lcd, int x, int y, char c)
{
    hd44780_gotoxy(lcd, x, y);
    hd44780_puts(lcd, (const char *) c);
}

/**
 * Writes a first a char(icon) and afterwards a string on the given postion
 * EXAMPLE: write_and_upload_char(&lcd, 0,0," Internet Radio", 0);
 * this example writes first the char(icon) 0 and then the string "Internet Radio" the output on the LCD is like this 
 * Output: C Internet Radio
*/
void write_and_upload_char(const hd44780_t *lcd, int x, int y, const char *string, char c)
{
    hd44780_gotoxy(lcd, x, y);
    hd44780_putc(lcd, c);
    hd44780_puts(lcd, string);
}