#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Define size of string arrays
#define FIRST_STRINGS_SIZE 4
#define SECOND_STRINGS_SIZE 3
#define THIRD_STRINGS_SIZE 4


int get_random_num(int min, int max);
void print_random_prediction(int index1, int index2, int index3);
void random_prediction(void* pvParameters);
