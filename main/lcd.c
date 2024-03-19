#include "init.h"
#include "sdcard_player.h"
#include "lcd.h"
#include "recorder.h"
#include "playlist.h"

const char *TAG = "LCD";

extern struct tm timeinfo;

typedef struct Element_Position
{
    int x;
    int y;
} Element_Position;

extern Element_Position element_position;
extern Element_Position page_position;

bool audio_mode_toggle = false;

/**
 * Variable for HD44780 I2C LCD configuration.
 */
extern hd44780_t lcd;

esp_err_t touchpad_handle(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    return lcd_touchpad_handle(evt, ctx);
}

/**
 * Initializes and displays a simple menu on the LCD.
 * @param pvParameters Pointer to task parameters (not used).
 */
void menu(void *pvParameters)
{
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
    uint8_t tuner[] = {0b00000, 0b00100, 0b01110, 0b10101, 0b10101, 0b10101, 0b11111, 0b11111};
    uint8_t internet_radio[] = {0b00000, 0b00110, 0b01001, 0b10001, 0b10101, 0b10001, 0b01001, 0b00110};
    uint8_t sampler[] = {0b00000, 0b01000, 0b11000, 0b01000, 0b01110, 0b01111, 0b01110, 0b01100};
    uint8_t menu_pages[] = {0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111};
    uint8_t current_page[] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};
    uint8_t arrow[] = {0b00000, 0b00100, 0b00010, 0b11111, 0b11111, 0b00010, 0b00100, 0b00000};
    uint8_t empty[] = {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};

    // Upload custom icons to LCD
    hd44780_upload_character(&lcd, 0, internet_radio);
    hd44780_upload_character(&lcd, 1, sampler);
    hd44780_upload_character(&lcd, 2, tuner);
    hd44780_upload_character(&lcd, 3, menu_pages);
    hd44780_upload_character(&lcd, 4, arrow);
    hd44780_upload_character(&lcd, 5, empty);
    hd44780_upload_character(&lcd, 6, current_page);

    page_position.x = 9;
    page_position.y = 0;

    // Writing the lines
    write_and_upload_char(1, 1, 0, " Internet Radio");
    write_and_upload_char(1, 2, 1, " Tijd");
    write_and_upload_char(1, 3, 2, " Weer");

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

// Handle touch pad events to control music playback and adjust volume
esp_err_t lcd_touchpad_handle(periph_service_event_t *evt, void *ctx)
{
    if (!audio_mode_toggle)
    {
        printf("INSIDE LCD TOUCHPAD HANDLE!!\n");
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
                audio_mode_toggle = true;
                mode_handle();
                break;
            case INPUT_KEY_USER_ID_REC:
                ESP_LOGW(TAG, "[ * ] [REC-] input key event");
                rec_handle();
                break;
            }
        }
    }
    else
    {
        printf("INSIDE SDCARD TOUCHPAD HANDLE!!\n");
        if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE)
        {
            ESP_LOGW(TAG, "[ * ] input key id is %d", (int)evt->data);

            switch ((int)evt->data)
            {
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGW(TAG, "[ * ] [Play] input key event");
                // play_button_handle();
                break;
            case INPUT_KEY_USER_ID_SET:
                ESP_LOGW(TAG, "[ * ] [Set] input key event");
                // set_button_handle();
                break;
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGW(TAG, "[ * ] [Vol+] input key event");
                // vol_up_handle();
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGW(TAG, "[ * ] [Vol-] input key event");
                // vol_down_handle();
                break;
            case INPUT_KEY_USER_ID_MODE:
                ESP_LOGW(TAG, "[ * ] [MODE-] input key event");
                // mode_handle();
                break;
            case INPUT_KEY_USER_ID_REC:
                ESP_LOGW(TAG, "[ * ] [REC-] input key event");
                rec_handle();
                break;
            }
        }
    }

    return ESP_OK;
}

void rec_handle()
{
    audio_mode_toggle = false;
    hd44780_clear(&lcd);
    page_position.x = 9;
    page_position.y = 0;
    write_and_upload_char(1, 1, 0, " Internet Radio");
    write_and_upload_char(1, 2, 1, " Tijd");
    write_and_upload_char(1, 3, 2, " Weer");

    for (int i = 0; i < 3; i++)
    {
        write_char_on_pos(i + 9, 0, 3);
    }
    write_char_on_pos(9, 0, 6);

    write_char_on_pos(0, 1, 4);
    create_input_key_service();
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
            write_and_upload_char(1, 2, 1, " Tijd");
            write_and_upload_char(1, 3, 2, " Weer");
        }
        else if (page_position.x == 10)
        {
            write_and_upload_char(1, 1, 0, " Opname");
            write_and_upload_char(1, 2, 1, " Audio Speler");
            write_and_upload_char(1, 3, 2, " Papagaai");
        }
        else if (page_position.x == 11)
        {
            write_and_upload_char(1, 1, 0, " Voorspelling");
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
            write_and_upload_char(1, 2, 1, " Tijd");
            write_and_upload_char(1, 3, 2, " Weer");
        }
        else if (page_position.x == 10)
        {
            write_and_upload_char(1, 1, 0, " Opname");
            write_and_upload_char(1, 2, 1, " Audio Speler");
            write_and_upload_char(1, 3, 2, " Papagaai");
        }
        else if (page_position.x == 11)
        {
            write_and_upload_char(1, 1, 0, " Voorspelling");
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
            break;
        case 2:
            app_init();
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Tijd");
                time_t now;
                time(&now);
                localtime_r(&now, &timeinfo);

                audio_mode_toggle = true;
                create_audio_elements();
                char* files[20];
                print_full_time(&timeinfo);
                get_filenames_based_on_time(files, &timeinfo);
                sdcard_playlist(files, "NL/", 20);
                audio_mode_toggle = false;
            break;
        case 3:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Weer");
            break;
        }
    }
    else if (page_position.x == 10)
    {
        switch (element_position.y)
        {
        case 1:
            app_init();
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Opname");
            create_recording_elements();
            //audio_mode_toggle = false;
            create_recording("eren.wav", 6);
            write_string_on_pos(2, 1, "Opname af");
            break;
        case 2: 
            app_init();
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Audio Speler");
            audio_mode_toggle = true;
            create_audio_elements();
            const char *files2[] = {"eren.wav"};
            sdcard_playlist(files2,"",1);
            audio_mode_toggle = false;
            break;
        case 3:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Papagaai");
            break;
        }
    }
    else if (page_position.x == 11)
    {
        switch (element_position.y)
        {
        case 1:
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "Voorspelling");
            break;
        case 2: // Empty page
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "");
            write_char_on_pos(0, 1, 4);
            break;
        case 3: // Empty page
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "");
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