#include "lcd.h"
#include "recorder.h"

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);

    xTaskCreatePinnedToCore(record_task, "record_task", 4096, NULL, 5, NULL, 1);
}

