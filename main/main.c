#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "sdkconfig.h"

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

void app_main(void)
{
    time_t now;
    struct tm timeinfo;
    custom_wifi_config wifi_config = {0};
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
    create_audio_elements();
    char* files[20];
    print_full_time(&timeinfo);
    get_filenames_based_on_time(files, &timeinfo);
    sdcard_playlist(files, 20);
    sdcard_player_stop();

    //xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 1, NULL);
    
    
    //const char* time_dirty[15];
    //get_filenames_based_on_time(time_dirty, &timeinfo);
    //sdcard_player_init();
    //sdcard_player_start();
    
    //xTaskCreate(init_radio, "radio_test", configMINIMAL_STACK_SIZE * 5, (void*)&wifi_config, 5, NULL);

    free(output_buffer);

}