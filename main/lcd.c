#include "lcd.h"

/**
 * Static variable for PCF8574 I2C GPIO expander configuration.
 * The PCF8574 is used to interface with the LCD display, allowing for
 * data transmission over I2C, enabling the control of the LCD's digital inputs.
 */
static i2c_dev_t pcf8574;

/**
 * Bitmaps for LCD display icons, each icon consists of 8 rows to match LCD segment rows.
 * - tuner: Icon for tuner.
 * - internet_radio: Icon for internet radio.
 * - sampler: Icon for sampler.
 */
const uint8_t tuner[] = {0b00000, 0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b11111, 0b11111};
const uint8_t internet_radio[] = {0b00000, 0b00110, 0b01001, 0b10001, 0b10101, 0b10001, 0b01001, 0b00110};
const uint8_t sampler[] = {0b00000, 0b01000, 0b11000, 0b01000, 0b01110, 0b01111, 0b01110, 0b01100};

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
 * Initializes and displays a simple menu on the LCD.
 * @param pvParameters Pointer to task parameters (not used).
 */
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

    // Clear LCD and display menu items
    hd44780_clear(&lcd);
    write_and_upload_char(&lcd, 0, 0, 0, " Internet Radio");
    write_and_upload_char(&lcd, 0, 1, 1, " Sampler");
    write_and_upload_char(&lcd, 0, 2, 2, " Tuner");

    // Keep task running
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void write_string_on_pos(const hd44780_t *lcd, int x, int y, const char *string)
{
    hd44780_gotoxy(lcd, x, y); // Move cursor to the specified coordinates
    hd44780_puts(lcd, string); // Output the specified string
}

void write_char_on_pos(const hd44780_t *lcd, int x, int y, char c)
{
    hd44780_gotoxy(lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(lcd, c);      // Output the specified character
}

void write_and_upload_char(const hd44780_t *lcd, int x, int y, char c, const char *string)
{
    hd44780_gotoxy(lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(lcd, c);      // Output the specified string
    hd44780_puts(lcd, string); // Output the specified character
}