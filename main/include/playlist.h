//
// Created by Yuichi on 12/03/2024.
//

#ifndef PLAYLIST_ATTEMPT_PLAYLIST_H
#define PLAYLIST_ATTEMPT_PLAYLIST_H

#include <stdio.h>
#include "time.h"

// Define the maximum and minimum numbers for valid input
#define MAX_NUM 60
#define MIN_NUM 0

/**
 * @brief Prints the text representation of a number based on its value.
 *
 * This function checks if the number is within a valid range and prints the corresponding
 * audio file name for the number. It handles numbers up to 13 and base ten numbers up to 60.
 *
 * @param num The number to be printed.
 */
void print_filenames_based_on_num(int num);

/**
 * @brief Prints the full time in text format.
 *
 * This function prints the hour, minute, and second in text format using the audio files
 * corresponding to each time component.
 *
 * @param timeinfo Pointer to the struct tm containing the time information.
 */
void print_full_time(struct tm *timeinfo);

/**
 * @brief Returns an array of strings representing the audio files for a given number.
 *
 * This function allocates memory for an array of strings and fills it with the audio file names
 * corresponding to the given number. It handles numbers up to 13 and base ten numbers up to 60.
 *
 * @param num The number for which the audio files are to be returned.
 * @return A pointer to the array of strings.
 */
char ** get_filenames_based_on_num(int num);

/**
 * @brief Returns an array of strings representing the audio files for the full time.
 *
 * This function combines the audio files for the hour, minute, and second into a single array
 * of strings, formatted as "het is", hour, "uur", minute, "minuten", second, "seconden".
 *
 * @param timeinfo Pointer to the struct tm containing the time information.
 * @return A pointer to the array of strings.
 */
char ** get_filenames_based_on_time(struct tm *timeinfo);

#endif //PLAYLIST_ATTEMPT_PLAYLIST_H
