#pragma once
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <hd44780.h>
#include <pcf8574.h>

// Defining Methods
void menu(void *pvParameters);
void write_string_on_pos(const hd44780_t *lcd, int x, int y, const char *string);
void write_and_upload_char(const hd44780_t *lcd, int x, int y, const char *string, char c);