#include "playlist.h"
#include "stdlib.h"

// Array of strings representing base numbers
const char *baseNumberStrings[] = {
        "0.wav", "1.wav", "2.wav", "3.wav", "4.wav", "5.wav", "6.wav",
        "7.wav", "8.wav", "9.wav", "10.wav", "11.wav", "12.wav", "13.wav"
};

// Array of strings representing base ten numbers
const char *baseTenNumberStrings[] = {
        "10.wav", "20.wav", "30.wav", "40.wav", "50.wav", "60.wav"
};

const char *en = "en.wav";
const char *hetIs = "het is.wav";
const char *uur = "uur.wav";
const char *invalidNumber = "bruh.wav";

/**
 * @brief Prints the text representation of a number based on its value.
 *
 * This function checks if the number is within a valid range and prints the corresponding
 * audio file name for the number. It handles numbers up to 13 and base ten numbers up to 60.
 *
 * @param num The number to be printed.
 */
void print_filenames_based_on_num(int num) {
    // Check if the number is within the valid range
    if (num < MIN_NUM || num > MAX_NUM) {
        printf("%s", invalidNumber);
        return;
    }

    // If the number is less than 14, it is one of the base numbers
    if (num < 14) {
        printf("%s ", baseNumberStrings[num]);
        return;
    }

    // If the number is greater than or equal to 14, calculate the base ten number and remainder
    int ten = (num / 10) - 1; // Calculate the index for the base ten number in the lookup table
    if ((num % 10) != 0) { // Print the remainder if it's not 0
        printf("%s ", baseNumberStrings[num % 10]);
        if (ten > 0)
            printf("%s ", en); // if the base ten number is greater than 0
    }
    printf("%s ", baseTenNumberStrings[ten]); // Print the base ten number
}

/**
 * @brief Prints the full time in text format.
 *
 * This function prints the hour, minute, and second in text format using the audio files
 * corresponding to each time component.
 *
 * @param timeinfo Pointer to the struct tm containing the time information.
 */
void print_full_time(struct tm *timeinfo) {
    printf("%s ", hetIs);
    print_filenames_based_on_num(timeinfo->tm_hour); // Print the hour
    printf("%s, ", uur);
    print_filenames_based_on_num(timeinfo->tm_min); // Print the minute
    printf("minuten, ");
    print_filenames_based_on_num(timeinfo->tm_sec); // Print the second
    printf("seconden");
}

/**
 * @brief Returns an array of strings representing the audio files for a given number.
 *
 * This function allocates memory for an array of strings and fills it with the audio file names
 * corresponding to the given number. It handles numbers up to 13 and base ten numbers up to 60.
 *
 * @param num The number for which the audio files are to be returned.
 * @return A pointer to the array of strings.
 */
char **get_filenames_based_on_num(int num) {
    char **file_array = malloc(3 * sizeof(char *)); // Allocate memory for the array of strings
    if (file_array == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Check if the number is within the valid range
    if (num < MIN_NUM || num > MAX_NUM) {
        // Set all elements to "invalid number"
        file_array[0] = invalidNumber;
        file_array[1] = NULL; // Set to NULL to indicate end of array
        return file_array;
    }

    // If the number is less than 14, it is one of the base numbers
    if (num < 14) {
        file_array[0] = baseNumberStrings[num];
        file_array[1] = NULL; // Set to NULL to indicate end of array
        return file_array;
    }

    // If the number is greater than or equal to 14, calculate the base ten number and remainder
    int ten = (num / 10) - 1; // Calculate the index for the base ten number in the lookup table
    int index = 0; // Index for the file_array
    if ((num % 10) != 0) { // Print the remainder if it's not 0
        file_array[index++] = baseNumberStrings[num % 10];
        if (ten > 0)
            file_array[index++] = en; // if the base ten number is greater than 0
    }
    file_array[index++] = baseTenNumberStrings[ten]; // Set the base ten number
    file_array[index] = NULL; // Set to NULL to indicate end of array

    return file_array;
}

/**
 * @brief Returns an array of strings representing the audio files for the full time.
 *
 * This function combines the audio files for the hour, minute, and second into a single array
 * of strings, formatted as "het is", hour, "uur", minute, "minuten", second, "seconden".
 *
 * @param timeinfo Pointer to the struct tm containing the time information.
 * @return A pointer to the array of strings.
 */
char ** get_filenames_based_on_time(struct tm *timeinfo) {
    char **full_file_array = malloc(8 * sizeof(char *)); // Allocate memory for the array of strings
    // Sizeof 8, voor "het is", nummer van max 3, "uur", nummer van max 3.
    if (full_file_array == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    char **hour_filenames = get_filenames_based_on_num(timeinfo->tm_hour);
    char **min_filenames = get_filenames_based_on_num(timeinfo->tm_min);

    int full_index = 0;
    full_file_array[full_index++] = hetIs;

    int i = 0;
    while (hour_filenames[i] != NULL) { // Combine full_file_array and hour_filenames
        full_file_array[full_index++] = hour_filenames[i++];
    }

    full_file_array[full_index++] = uur;

    i = 0;
    while (min_filenames[i] != NULL) { // Combine full_file_array and min_filenames
        full_file_array[full_index++] = min_filenames[i++];
    }
    full_file_array[full_index] = NULL; // Set NULL to indicate end of array

    return full_file_array;
}
