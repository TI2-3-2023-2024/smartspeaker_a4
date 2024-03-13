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

static const char* TAG = "MAIN";

void app_main(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }

    ESP_ERROR_CHECK(i2cdev_init());

    xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 1, NULL);
    
    xTaskCreate(init_radio, "radio_test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);

}
