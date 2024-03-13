#include "timesync.h"

// Define a tag for logging purposes
static const char * TAG = "Timesync";

/**
 * @brief Callback function for time synchronization notification.
 *
 * This function logs a message indicating a time synchronization event has occurred.
 *
 * @param tv Pointer to a timeval structure containing the new time.
 */
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

/**
 * @brief Obtains the current time by initializing necessary components and setting up SNTP.
 *
 * This function initializes NVS, network interface, event loop, and optionally configures NTP server
 * address via DHCP. It then connects to the network, initializes SNTP, and waits for the system time to be set.
 * Finally, it disconnects from the network.
 */
void obtain_time(void)
{
    // Initialization steps
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK( esp_event_loop_create_default());

    // Optional: Configure NTP server address via DHCP
#if LWIP_DHCP_GET_NTP_SRV
    esp_sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
#endif

    // Connect to network
    ESP_ERROR_CHECK(example_connect());

    // Initialize SNTP
    initialize_sntp();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Set timezone
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    // Disconnect from network
    ESP_ERROR_CHECK( example_disconnect());
}

/**
 * @brief Initializes the SNTP client with a specified operating mode and server.
 *
 * This function sets the SNTP client to poll mode, specifies the NTP server, and sets a callback
 * function for time synchronization notifications. It also configures the sync mode if smooth sync
 * is enabled.
 */
void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    esp_sntp_init();
}
