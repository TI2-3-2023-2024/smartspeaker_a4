#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @file rnd_prediction.c
 * @brief Implements functions for generating random predictions and filenames.
 *
 * This file contains functions for generating random predictions based on predefined
 * groups of filenames and for generating random numbers within a specified range.
 *
 * @author Moustapha Azaimi
 * @date 2024-03-19
 */

/**
 * @brief Fills an array with filenames based on a random prediction.
 *
 * This function selects random filenames from predefined groups and fills the provided
 * array with these filenames. The array is expected to be large enough to hold all filenames.
 *
 * @param files Pointer to an array of char pointers where the filenames will be stored.
 * @param files_size The size of the files array.
 */
void get_filenames_based_on_prediction(char **files, size_t files_size);

/**
 * @brief Generates a random number within a specified range.
 *
 * This function uses the ESP32's hardware random number generator to produce a random
 * number within the specified range.
 *
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return A random number within the range [min, max].
 */
int get_random_num(int min, int max);
