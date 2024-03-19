#pragma once
#include "custom_wifi.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "parson.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

typedef struct weer_info{
    char location[30];
    char status[30];
    float temperature;
} weer_info;

static char* json_test = "{ \"liveweer\": [{\"plaats\": \"Breda\", \"timestamp\": 1710761884, \"time\": \"18-03-2024 12:38:04\", \"temp\": \"14.4\", \"gtemp\": 12.9, \"samenv\": \"Half bewolkt\", \"lv\": 59, \"windr\": \"WNW\", \"windrgr\": 286.8, \"windms\": 3.19, \"windbft\": 2, \"windknp\": 6.2, \"windkmh\": 11.5, \"luchtd\": 1016.44, \"ldmmhg\": 762, \"dauwp\": 6.4, \"zicht\": 27500, \"gr\": 514, \"verw\": \"Van het westen uit zonnige perioden, een enkele bui\", \"sup\": \"06:44\", \"sunder\": \"18:51\", \"image\": \"halfbewolkt\", \"alarm\": 0, \"lkop\": \"Er zijn geen waarschuwingen\", \"ltekst\": \"Er zijn momenteel geen waarschuwingen van kracht.\", \"wrschklr\": \"groen\", \"wrsch_g\": \"-\", \"wrsch_gts\": 0, \"wrsch_gc\": \"-\"}]}";
void RequestWeather(char* location, char* status, float* temp, char* output_buffer);
void ParseJSON(char* location, char* status, float* temp, char* output_buffer);