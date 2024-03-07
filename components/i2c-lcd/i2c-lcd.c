#include "i2c-lcd.h"

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
    hd44780_upload_character(&lcd, 0, internet_radio);
    hd44780_upload_character(&lcd, 1, sampler);
    hd44780_upload_character(&lcd, 2, tuner);

    // Clear LCD
    hd44780_clear(&lcd);

    // Writing something
    write_and_upload_char(&lcd, 0, 0, " Internet Radio", 0);
    write_and_upload_char(&lcd, 0, 1, " Sampler", 1);
    write_and_upload_char(&lcd, 0, 2, " Tuner", 2);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    hd44780_puts(lcd, c);
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