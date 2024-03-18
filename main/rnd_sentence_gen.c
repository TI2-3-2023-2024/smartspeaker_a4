#include "rnd_sentence_gen.h"
#include "sdcard_player.h"
#include <stdio.h>

// Define size of string arrays
#define FIRST_STRINGS_SIZE 3
#define SECOND_STRINGS_SIZE 1
#define THIRD_STRINGS_SIZE 3
#define FOURTH_STRINGS_SIZE 2

// Define string arrays containing sound fragments
const char* first_strings[FIRST_STRINGS_SIZE] = {
    "later",
    "vandaag",
    "nu"
};

const char* second_strings[SECOND_STRINGS_SIZE] = {
    "wordt een"
};

const char* third_strings[THIRD_STRINGS_SIZE] = {
    "zonnige",
    "bewolkte",
    "frusterende"
};

const char* fourth_strings[FOURTH_STRINGS_SIZE] = {
    "dag",
    "nacht"
};

// Generate random number within range
int getRandomNumber(int min, int max) {
    return min + rand() % (max - min + 1);
}

void random_sentence(void *pvParameters) {
    while(1) {
        // Generate random indices for each string array
        int index1 = getRandomNumber(0, FIRST_STRINGS_SIZE - 1);
        int index2 = getRandomNumber(0, SECOND_STRINGS_SIZE - 1);
        int index3 = getRandomNumber(0, THIRD_STRINGS_SIZE - 1);
        int index4 = getRandomNumber(0, FOURTH_STRINGS_SIZE - 1);

        // Play each word separately with delays
    //     play_sound_by_filename(first_strings[index1]);
    //     play_sound_by_filename(second_strings[index2]);
    //     play_sound_by_filename(third_strings[index3]);
    //     play_sound_by_filename(fourth_strings[index4]);
    // }

    char* playlist = {first_strings[index1], second_strings[index2], third_strings[index3], fourth_strings[index4]}, NULL;
    }
}

