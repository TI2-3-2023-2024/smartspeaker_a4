#pragma once
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"

#include "esp_peripherals.h"
#include "periph_wifi.h"
#include "board.h"
#include "custom_wifi.h"

#include "esp_netif.h"

//radio kanalen kunnen toegevoegd worden.
static const char* radio_streams[3] = {
    "https://www.mp3streams.nl/zender/radio-538/stream/4-mp3-128", 
    "https://www.mp3streams.nl/zender/qmusic-non-stop/stream/125-mp3-96", 
    "https://www.mp3streams.nl/zender/concertzender-klassiek/stream/110-mp3-128"};

void init_radio(void* arg);