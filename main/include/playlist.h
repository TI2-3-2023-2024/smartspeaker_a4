//
// Created by Yuich on 12/03/2024.
//

#ifndef PLAYLIST_ATTEMPT_PLAYLIST_H
#define PLAYLIST_ATTEMPT_PLAYLIST_H

#include <stdio.h>
#include "time.h"

// Define the maximum and minimum numbers for valid input
#define MAX_NUM 60
#define MIN_NUM 0

// Function to print the text representation of a number
// Parameters:
// - num: The number to be printed
void print_filenames_based_on_num(int num);

// Function to print the full time in text
// Parameters:
// - timeinfo: Pointer to the struct tm containing the time information
void print_full_time(struct tm *timeinfo);

char ** get_filenames_based_on_num(int num);
char ** get_filenames_based_on_time(struct tm *timeinfo);

#endif //PLAYLIST_ATTEMPT_PLAYLIST_H
