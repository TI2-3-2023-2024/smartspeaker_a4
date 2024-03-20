#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "sdkconfig.h"
#include "math.h"

#include "lcd.h"
#include "radio.h"
#include "timesync.h"
#include "playlist.h"
#include "sdcard_player.h"
#include "esp_http_client.h"
#include "custom_wifi.h"
#include "weer.h"
#include "frequency_detect.h"

static const char* TAG = "MAIN";
struct tm timeinfo;

static void my_timer(void *pvParameters);
static void my_alarm(void *pvParameters);
static void my_stopwatch(void *pvParameters);

custom_wifi_config wifi_config = {0};

void app_main(void) {
    time_t now;
    
    weer_info weer;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time(&wifi_config);
        // update 'now' variable with current time
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
    // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
    init_wifi_nvs(&wifi_config);
    connect_wifi(&wifi_config);

    char* output_buffer = (char*)calloc(MAX_HTTP_OUTPUT_BUFFER+1, sizeof(char));   // Buffer to store response of http request
    RequestWeather(weer.location, &weer.temperature, output_buffer);
        
    printf("%s", weer.location);
    printf("%f", weer.temperature);

    app_init();

    xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 1, NULL);

    
    // xTaskCreate(init_radio, "radio_test", configMINIMAL_STACK_SIZE * 5, (void*)&wifi_config, 5, NULL);

    free(output_buffer);

    //xTaskCreatePinnedToCore(init_radio, "radio_test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL, 0);

    //xTaskCreatePinnedToCore(my_stopwatch, "logtime", configMINIMAL_STACK_SIZE * 5, NULL, 6, NULL, 1);
}

static void my_alarm(void *pvParameters) {

    ESP_LOGI(TAG, "Time-logging started");

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[10];

    strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

    char current_minute = strftime_buf[4];
    char previous_minute = current_minute;

    char current_second = strftime_buf[7];
    char previous_second = current_second;

    for (;;) {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

        if (current_minute != previous_minute) {
            ESP_LOGI(TAG, "The current_minute time is: %s", strftime_buf);
            previous_minute = current_minute;
        }

        current_minute = strftime_buf[4];
        current_second = strftime_buf[7];

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void my_timer(void* pvParameters) {

    ESP_LOGI(TAG, "Time-logging started");

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[10];

    strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

    char current_minute = strftime_buf[4];
    char previous_minute = current_minute;

    char current_second = strftime_buf[7];
    char previous_second = current_second;

    int timer_in_seconds = 93;
    int timer_count_down = timer_in_seconds;
    bool timer_active = true;
    int timer_minutes;
    int timer_seconds;

    for (;;) {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

        if (timer_active && current_second != previous_second) {
            timer_seconds = timer_count_down % 60;
            timer_minutes = floor(timer_count_down / 60);

            ESP_LOGI(TAG, "Time left on timer: %02d:%02d", timer_minutes, timer_seconds);
            timer_count_down--;
            previous_second = current_second;
        }

        if (timer_count_down == 0 && timer_active) {
            ESP_LOGI(TAG, "Times up!");
            timer_active = false;
        }

        current_minute = strftime_buf[4];
        current_second = strftime_buf[7];

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void my_stopwatch(void* pvParameters) {
    ESP_LOGI(TAG, "Time-logging started");

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[10];

    strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

    char current_minute = strftime_buf[4];
    char previous_minute = current_minute;

    char current_second = strftime_buf[7];
    char previous_second = current_second;

    int timer_minutes;
    int timer_seconds;
    int timer = 0;

    for (;;) {
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%X", &timeinfo);

        if (current_second != previous_second) {
            timer_seconds = timer % 60;
            timer_minutes = floor(timer / 60);

            ESP_LOGI(TAG, "Time elapsed: %02d:%02d", timer_minutes, timer_seconds);

            timer++;
            previous_second = current_second;
        }

        current_minute = strftime_buf[4];
        current_second = strftime_buf[7];

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}