// naive code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define MAX_WORD_LENGTH 120
#define MAX_WORDS 18000000
#define MAX_FREQ_WORDS 260000

typedef struct {
    char *word;
    int frequency;
} WordFrequency;

char **read_file(char *filename, int *count) {
    char buffer[MAX_WORD_LENGTH];
    *count = 0;
    char **words = malloc(MAX_WORDS * sizeof(char *));
    if (words == NULL) {
        printf("Error: Memory allocation for words array failed.\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("File not found: %s.\n", filename);
        free(words);
        return NULL;
    }

    while (fscanf(file, "%s", buffer) != EOF && *count < MAX_WORDS) {
        if (strlen(buffer) > 0 && strlen(buffer) < MAX_WORD_LENGTH) {
            words[*count] = malloc(strlen(buffer) + 1);
            if (words[*count] == NULL) {
                printf("Error: Memory allocation failed for word %d.\n", *count);
                break;
            }
            strcpy(words[*count], buffer);
            (*count)++;
        }
    }

    fclose(file);

    if (*count == 0) {
        printf("No words were read from the file.\n");
        free(words);
        return NULL;
    }

    return words;
}

int compare_words(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

WordFrequency *create_frequency_array(char **words, int word_count, int *freq_count) {
    WordFrequency *freq_array = malloc(MAX_FREQ_WORDS * sizeof(WordFrequency));
    if (freq_array == NULL) {
        printf("Error: Memory allocation for frequency array failed.\n");
        return NULL;
    }

    qsort(words, word_count, sizeof(char *), compare_words);

    *freq_count = 0;

    for (int i = 0; i < word_count; i++) {
        if (*freq_count == 0 || strcmp(words[i], freq_array[*freq_count - 1].word) != 0) {
            if (*freq_count < MAX_FREQ_WORDS) {
                freq_array[*freq_count].word = words[i];
                freq_array[*freq_count].frequency = 1;
                (*freq_count)++;
            }
        } else {
            freq_array[*freq_count - 1].frequency++;
        }
    }

    return freq_array;
}

int compare_frequency(const void *a, const void *b) {
    WordFrequency *wordA = (WordFrequency *)a;
    WordFrequency *wordB = (WordFrequency *)b;
    return wordB->frequency - wordA->frequency;
}

void free_words(char **words, int count) {
    for (int i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}

int main() {
    struct timeval start, end;
    double elapsed, total_elapsed;

    // Record the start time of the entire program
    gettimeofday(&start, NULL);

    char *filename = "large_file.txt";
    int word_count = 0;

    // Time for reading the file
    struct timeval intermediate_start, intermediate_end;
    gettimeofday(&intermediate_start, NULL);
    char **words = read_file(filename, &word_count);
    gettimeofday(&intermediate_end, NULL);
    elapsed = (intermediate_end.tv_sec - intermediate_start.tv_sec) + 
              (intermediate_end.tv_usec - intermediate_start.tv_usec) / 1000000.0;
    printf("Time to read the file: %f seconds\n", elapsed);

    if (words != NULL) {
        int freq_count = 0;

        // Time for creating the frequency array
        gettimeofday(&intermediate_start, NULL);
        WordFrequency *freq_array = create_frequency_array(words, word_count, &freq_count);
        gettimeofday(&intermediate_end, NULL);
        elapsed = (intermediate_end.tv_sec - intermediate_start.tv_sec) + 
                  (intermediate_end.tv_usec - intermediate_start.tv_usec) / 1000000.0;
        printf("Time to create the frequency array: %f seconds\n", elapsed);

        if (freq_array != NULL) {
            // Time for sorting the frequency array
            gettimeofday(&intermediate_start, NULL);
            qsort(freq_array, freq_count, sizeof(WordFrequency), compare_frequency);
            gettimeofday(&intermediate_end, NULL);
            elapsed = (intermediate_end.tv_sec - intermediate_start.tv_sec) + 
                      (intermediate_end.tv_usec - intermediate_start.tv_usec) / 1000000.0;

            // Display the top 10 most frequent words
            printf("\nTop 10 most frequent words:\n");
            for (int i = 0; i < 10 && i < freq_count; i++) {
                printf("Word: %s, Frequency: %d\n", freq_array[i].word, freq_array[i].frequency);
            }

            free(freq_array);
        }

        free_words(words, word_count);
    }

    // Record the end time of the entire program
    gettimeofday(&end, NULL);
    total_elapsed = (end.tv_sec - start.tv_sec) + 
                    (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("\nTotal execution time of the program: %f seconds\n", total_elapsed);

    return 0;
}
