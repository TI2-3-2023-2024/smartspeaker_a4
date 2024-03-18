#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <stdio.h>
#include "sdcard_player.h"


int getRandomNumber(int min, int max);
void random_sentence(void *pvParameters);
