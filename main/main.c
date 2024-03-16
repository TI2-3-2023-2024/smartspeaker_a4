#include "init.h"
#include "lcd.h"
#include "recorder.h"
#include "sdcard_player.h"

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}

