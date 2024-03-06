idf_component_register(SRCS "wifi_radio.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash audio_pipeline audio_stream esp_peripherals esp-adf-libs)