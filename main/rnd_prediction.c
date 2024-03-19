#include "rnd_prediction.h"

// Define string arrays containing sound file names
const char* group_one[FIRST_STRINGS_SIZE] = {
    "later.wav",
    "vandaag.wav",
    "nu.wav",
    "morgen.wav"
};

const char* wordt = "wordt.wav";
const char* een = "een.wav";

const char* group_two[SECOND_STRINGS_SIZE] = {
    "zonnige.wav",
    "bewolkte.wav",
    "goede.wav"
};

const char* group_three[THIRD_STRINGS_SIZE] = {
    "dag.wav",
    "nacht.wav",
    "avond.wav",
    "ochtend.wav"
};

/**
 * @brief Prints the full random sentence in text format.
 *
 * This function prints the sentence in text format using the audio files
 * corresponding to randomized parts.
 *
 * @param index1 Pointer to the index for the string array group_one.
 * @param index2 Pointer to the index for the string array group_two.
 * @param index3 Pointer to the index for the string array group_three.
 */
void print_random_prediction(int index1, int index2, int index3) {
    printf("%s ", group_one[index1]);
    printf("%s ", wordt);
    printf("%s ", een);
    printf("%s ", group_two[index2]);
    printf("%s ", group_three[index3]);
}

// Generate a random number within a certain boundary
int get_random_num(int min, int max) {
    return min + esp_random() % (max - min + 1);
}

void random_prediction(void* pvParameters) {
    int index1 = get_random_num(0, FIRST_STRINGS_SIZE - 1);
    int index2 = get_random_num(0, SECOND_STRINGS_SIZE - 1);    
    int index3 = get_random_num(0, THIRD_STRINGS_SIZE - 1);

    print_random_prediction(index1, index2, index3);

    vTaskDelete(NULL);
}





