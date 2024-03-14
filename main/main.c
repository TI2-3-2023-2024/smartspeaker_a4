#include "lcd.h"
#include "recorder.h"
#include "sdcard_player.h"

void record_play_task(void *pvParameters);

void app_main()
{
    // ESP_ERROR_CHECK(i2cdev_init());
    // xTaskCreate(menu, "lcd_test", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    sdcard_player_init();
    xTaskCreatePinnedToCore(sdcard_player_start, "record_task", 4096, NULL, 5, NULL, 1);
    // xTaskCreatePinnedToCore(record_task, "record_task", 4096, NULL, 5, NULL, 1);
    // xTaskCreatePinnedToCore(record_play_task, "record_task", 4096, NULL, 5, NULL, 1);
}

void record_play_task(void *pvParameters)
{
    bool isFinished = false;
    sdcard_player_init();

    while (isFinished == false)
    {
        recorder_init(16000);
        printf("record started\n");
        create_recording("eren.wav", 5);
        isFinished = true;
        printf("inside if and bool is %d\n", isFinished);
        break;
    }

    play_sound_by_filename("eren.wav");

    printf("finished loop\n");
    vTaskDelay(10000 / portTICK_PERIOD_MS);
}
