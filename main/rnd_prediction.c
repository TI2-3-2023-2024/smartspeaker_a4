#include "rnd_prediction.h"

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

// Define size of string arrays
#define FIRST_STRINGS_SIZE 4
#define SECOND_STRINGS_SIZE 3
#define THIRD_STRINGS_SIZE 4

// Define string arrays containing sound file names
const char *group_one[FIRST_STRINGS_SIZE] = {
    "later.wav",
    "vandaag.wav",
    "nu.wav",
    "morgen.wav"};

const char *group_two[SECOND_STRINGS_SIZE] = {
    "zonnige.wav",
    "bewolkte.wav",
    "goede.wav"};

const char *group_three[THIRD_STRINGS_SIZE] = {
    "dag.wav",
    "nacht.wav",
    "avond.wav",
    "ochtend.wav"};

/**
 * @brief Fills an array with filenames based on a random prediction.
 *
 * This function selects random filenames from predefined groups and fills the provided
 * array with these filenames. The array is expected to be large enough to hold all filenames.
 *
 * @param files Pointer to an array of char pointers where the filenames will be stored.
 * @param files_size The size of the files array.
 */
void get_filenames_based_on_prediction(char **files, size_t files_size)
{
    int index1 = get_random_num(0, FIRST_STRINGS_SIZE - 1);
    int index2 = get_random_num(0, SECOND_STRINGS_SIZE - 1);
    int index3 = get_random_num(0, THIRD_STRINGS_SIZE - 1);

    files[0] = (char *)group_one[index1];
    files[1] = "wordt.wav";
    files[2] = "een.wav";
    files[3] = (char *)group_two[index2];
    files[4] = (char *)group_three[index3];

    for (size_t i = 5; i < files_size; i++)
    {
        files[i] = NULL;
    }
}

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
int get_random_num(int min, int max)
{
    return min + esp_random() % (max - min + 1);
}
