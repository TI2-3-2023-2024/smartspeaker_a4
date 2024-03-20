#include "init.h"
#include "sdcard_player.h"
#include "lcd.h"
#include "recorder.h"
#include "frequency_detect.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)         \
    ((byte) & 0x10 ? '1' : '0'),     \
        ((byte) & 0x08 ? '1' : '0'), \
        ((byte) & 0x04 ? '1' : '0'), \
        ((byte) & 0x02 ? '1' : '0'), \
        ((byte) & 0x01 ? '1' : '0')

const char *TAG = "LCD";

typedef struct Element_Position
{
    int x;
    int y;
} Element_Position;

extern Element_Position element_position;
extern Element_Position page_position;

TaskHandle_t frequencyTaskHandle = NULL;

bool audio_mode_toggle = false;

/**
 * Variable for HD44780 I2C LCD configuration.
 */
extern hd44780_t lcd;

esp_err_t touchpad_handle(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    return lcd_touchpad_handle(evt, ctx);
}

void display_percentage(int y, int percentage)
{
    const int amountOfDisplays = 20;
    const char fullBlockID = 6;

    int fullBlocks = percentage / 5;
    int rest = percentage % 5;

    char pattern = 0b11111;
    pattern <<= (5 - rest);

    uint8_t customPattern[] = {pattern, pattern, pattern, pattern, pattern, pattern, pattern, pattern};
    char customPatternID = 7;
    hd44780_upload_character(&lcd, customPatternID, customPattern);

    for (int x = 0; x < amountOfDisplays; x++)
    {
        if (x < fullBlocks)
            write_char_on_pos(x, y, fullBlockID);
        else if (x == fullBlocks)
            write_char_on_pos(x, y, customPatternID);
        else
            clear_at_position(x, y);
    }
    element_position.x = 0;
    element_position.y = 1;
    page_position.x = 9;
    page_position.y = 0;
}

void updateVolume(VolumeMeter *volume, char value)
{
    if (value < 0)
        value = 0;
    else if (value > 100)
        value = 100;

    volume->firstBar -= 8;
    volume->secondBar -= 4;

    if (volume->firstBar > 200)
        volume->firstBar = 0; // overflow
    if (volume->secondBar > 200)
        volume->secondBar = 0; // overflow

    if (volume->firstBar < value)
        volume->firstBar = value;
    if (volume->secondBar < value)
    volume->secondBar = value;
}

void displayVolume(VolumeMeter *v, int y)
{

    display_percentage(y, v->firstBar);
    display_percentage(y + 1, v->secondBar);
}
SemaphoreHandle_t semaphore;

void setupCustomChars() {
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
}

/**
 * Initializes and displays a simple menu on the LCD.
 * @param pvParameters Pointer to task parameters (not used).
 */
void menu(void *pvParameters)
{
    semaphore = xSemaphoreCreateMutex();
    
    setupCustomChars();
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
    printf("\nPAGE_POSITION: X: %i\tY: %i\n", page_position.x, page_position.y);
    printf("ELEMENT_POSITION: X: %i\tY: %i\n", element_position.x, element_position.y);



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
    setupCustomChars();
    if (frequencyTaskHandle != NULL)
    {
        vTaskDelay(250 / portTICK_RATE_MS);
        ESP_LOGE("A", "STOPPING FREQUENCY TASK!");
        stop_goertzel_detection();
        // vTaskDelete(frequencyTaskHandle);
        // vTaskDelay(250 / portTICK_RATE_MS);
        ESP_LOGE("A", "STOPPED FREQUENCY TASK!");
        // frequencyTaskHandle = NULL;
    }
    stop_goertzel_detection();
    
    vTaskDelay(500 / portTICK_RATE_MS);

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
    // create_input_key_service();
}

void play_button_handle()
{
    if (element_position.y < 3)
    {
        clear_at_position(element_position.x, element_position.y);
        element_position.y += 1;
        write_char_on_pos(element_position.x, element_position.y, 4);
    } else
        element_position.y = 3;
}

void set_button_handle()
{
    if (element_position.y > 1)
    {
        clear_at_position(element_position.x, element_position.y);
        element_position.y -= 1;
        write_char_on_pos(element_position.x, element_position.y, 4);
    } else {
        element_position.y = 1;
    }
}

void vol_up_handle()
{
    if (page_position.x > 11)
        page_position.x = 11;
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
            write_and_upload_char(1, 2, 0, " Frequency");
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
            write_and_upload_char(1, 2, 0, " Frequency");
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
            create_audio_elements();
            const char *files[] = {"rec.wav"};
            sdcard_playlist(files, 1);
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
            // audio_mode_toggle = false;
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
            sdcard_playlist(files2, 1);
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
            app_init();
            // ESP_LOGE("A", "CLEARING LCD");
            hd44780_clear(&lcd);
            // ESP_LOGE("A", "CREATING ALT RECORDING ELEMENTS");
            // create_alt_recording_elements();
            // ESP_LOGE("A", "WRITING TO LCD");
            write_string_on_pos(0, 0, "Frequency");
            // write_char_on_pos(0, 1, 4);
            ESP_LOGE("A", "STARTING GROETZEL");
            init_goertzel_detector();
            start_goertzel_detection();

            // ESP_LOGE("A", "STARTING FREQUENCY TASK!");
            // xTaskCreate(frequency_detection_task, "FREQUENCY", configMINIMAL_STACK_SIZE * 5, NULL, 5, frequencyTaskHandle);
            // ESP_LOGE("A", "STARTED FREQUENCY TASK!");
            ESP_LOGE("A", "DONE GOERTZEL");
            app_init();

            break;
        case 3: // Empty page
            hd44780_clear(&lcd);
            write_string_on_pos(0, 0, "");
            write_char_on_pos(0, 1, 4);
            break;
        }
    }
}

void validateElementPosition() {
    element_position.x = 0;
    if (element_position.y > 3)
        element_position.y = 3;
}

void write_string_on_pos(int x, int y, const char *string)
{
    xSemaphoreTake(semaphore, portMAX_DELAY);
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_puts(&lcd, string); // Output the specified string
    validateElementPosition();
    xSemaphoreGive(semaphore);
}

void write_char_on_pos(int x, int y, char c)
{
    xSemaphoreTake(semaphore, portMAX_DELAY);
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, c);      // Output the specified character
    validateElementPosition();
    xSemaphoreGive(semaphore);
}

void write_and_upload_char(int x, int y, char c, const char *string)
{
    xSemaphoreTake(semaphore, portMAX_DELAY);
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, c);      // Output the specified string
    hd44780_puts(&lcd, string); // Output the specified character
    validateElementPosition();
    xSemaphoreGive(semaphore);
}

void clear_at_position(int x, int y)
{
    xSemaphoreTake(semaphore, portMAX_DELAY);
    element_position.x = x;
    element_position.y = y;
    hd44780_gotoxy(&lcd, x, y); // Move cursor to the specified coordinates
    hd44780_putc(&lcd, 5);      // Output the specified string
    validateElementPosition();
    xSemaphoreGive(semaphore);
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
    validateElementPosition();
}