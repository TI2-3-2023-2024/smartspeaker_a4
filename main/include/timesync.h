#pragma once

// Include necessary libraries
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
#include "custom_wifi.h"

/**
 * @brief Callback function for time synchronization notification.
 *
 * This function is called when the time is synchronized with the SNTP server.
 *
 * @param tv Pointer to a timeval structure containing the new time.
 */
void time_sync_notification_cb(struct timeval *tv);

/**
 * @brief Obtains the current time.
 *
 * This function initiates the process to obtain the current time from the SNTP server.
 */
void obtain_time(custom_wifi_config* config);

/**
 * @brief Initializes the SNTP client.
 *
 * This function sets up the SNTP client to synchronize time with the server.
 */
void initialize_sntp(void);
