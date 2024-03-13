#pragma once
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "lcd.h"
#include "radio.h"
#include "sdkconfig.h"
#include "protocol_examples_common.h"

void time_sync_notification_cb(struct timeval *tv);

void obtain_time(void);

void initialize_sntp(void);
