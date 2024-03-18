#include "weer.h"

const static char* TAG = "WEATHER";

void RequestWeather(char* location, float* temp, char* output_buffer){
    // int content_length = 0;
    // esp_http_client_config_t config = {
    //     .url = "https://weerlive.nl/api/json-data-10min.php?key=4b78afd2d0&locatie=Breda",
    // };
    // esp_http_client_handle_t client = esp_http_client_init(&config);

    // // GET Request
    // esp_http_client_set_method(client, HTTP_METHOD_GET);
    // esp_err_t err = esp_http_client_open(client, 0);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    // } else {
    //     content_length = esp_http_client_fetch_headers(client);
    //     if (content_length < 0) {
    //         ESP_LOGE(TAG, "HTTP client fetch headers failed");
    //     } else {
    //         int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
    //         if (data_read >= 0) {
    //             ESP_LOGI(TAG, "Received weather data");
    //         } else {    
    //             ESP_LOGE(TAG, "Failed to read response");
    //         }
    //     }
    // }
    // esp_http_client_close(client);
    // printf("%s", output_buffer);
    ParseJSON(location, temp, json_test);
}


void ParseJSON(char* location, float* temp, char* output_buffer){
    JSON_Value *root_value;
    JSON_Object *root_object; 
    JSON_Array *weather_array;
    JSON_Object *weather_array_object;
    root_value = json_parse_string(output_buffer);
    if (root_value == NULL) {
        printf("Error parsing JSON\n");
    }else{        
        root_object = json_value_get_object(root_value);
        weather_array = json_object_get_array(root_object, "liveweer");
        weather_array_object = json_array_get_object(weather_array, 0);
        strcpy(location, json_object_get_string(weather_array_object, "plaats"));
        *temp = (float)json_object_get_number(weather_array_object, "temp");
        json_value_free(root_value);  
    }
    
}