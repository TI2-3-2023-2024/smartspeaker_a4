#include "custom_wifi.h"

void init_wifi_nvs(custom_wifi_config* config){
    if(!config->nvs_initialized){
        // Initialization steps
        ESP_ERROR_CHECK( nvs_flash_init() );
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK( esp_event_loop_create_default());

        // Optional: Configure NTP server address via DHCP
        #if LWIP_DHCP_GET_NTP_SRV
            esp_sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
        #endif
        config->nvs_initialized = 1;
    }
}

void connect_wifi(custom_wifi_config* config){
    if(!config->initialized){
        example_connect();
        config->initialized = 1;
    }
}

void disconnect_wifi(custom_wifi_config* config){
    if(config->initialized){
        example_disconnect();
        config->initialized = 0;
    }
}