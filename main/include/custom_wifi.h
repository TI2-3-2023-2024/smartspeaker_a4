// Include necessary libraries
#pragma once
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_system.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "sdkconfig.h"
#include "protocol_examples_common.h"

typedef struct custom_wifi_config{
    unsigned char nvs_initialized;
    unsigned char initialized;
} custom_wifi_config;


void init_wifi_nvs(custom_wifi_config* config);
void connect_wifi(custom_wifi_config* config);
void disconnect_wifi(custom_wifi_config* config);