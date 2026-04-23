//multithreading code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_WORD_LENGTH 120
#define MAX_WORDS 18000000
#define MAX_FREQ_WORDS 260000

typedef struct {
    char *word;
    int frequency;
} WordFrequency;

typedef struct {
    char **words;
    int start;
    int end;
    WordFrequency *freq_array;
    int *freq_count;  // Each thread will maintain its own count of unique words
} ThreadArgs;

// Function to read the file and store words
char **read_file(char *filename, int *count) {
    char buffer[MAX_WORD_LENGTH];
    *count = 0;
    char **words = malloc(MAX_WORDS * sizeof(char *));
    if (words == NULL) {
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        free(words);
        return NULL;
    }

    while (fscanf(file, "%s", buffer) != EOF && *count < MAX_WORDS) {
        if (strlen(buffer) > 0 && strlen(buffer) < MAX_WORD_LENGTH) {
            words[*count] = malloc(strlen(buffer) + 1);
            if (words[*count] == NULL) {
                break;
            }
            strcpy(words[*count], buffer);
            (*count)++;
        }
    }

    fclose(file);
    return words;
}

// Function to process chunks of words in each thread
void *process_chunk(void *args) {
    ThreadArgs *data = (ThreadArgs *)args;
    char **words = data->words;
    int start_idx = data->start;
    int end_idx = data->end;
    WordFrequency *freq_array = data->freq_array;
    int *freq_count = data->freq_count;

    // Local frequency map for this chunk
    for (int i = start_idx; i < end_idx; i++) {
        int found = 0;
        for (int j = 0; j < *freq_count; j++) {
            if (strcmp(words[i], freq_array[j].word) == 0) {
                freq_array[j].frequency++;
                found = 1;
                break;
            }
        }

        if (!found && *freq_count < MAX_FREQ_WORDS) {
            freq_array[*freq_count].word = words[i];
            freq_array[*freq_count].frequency = 1;
            (*freq_count)++;
        }
    }

    pthread_exit(NULL);
}

// Function to compare frequencies of words
int compare_frequency(const void *a, const void *b) {
    WordFrequency *wordA = (WordFrequency *)a;
    WordFrequency *wordB = (WordFrequency *)b;
    return wordB->frequency - wordA->frequency;
}

// Function to compare words for sorting
int compare_words(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to free memory allocated for words
void free_words(char **words, int count) {
    for (int i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);
}

int main() {
    struct timeval start, end;
    double elapsed;
    gettimeofday(&start, NULL);
    int num_threads = 8;  // Use 2 threads or 4 or 6 or 8
    pthread_t threads[num_threads];
    char *filename = "large_file.txt";
    int word_count = 0;
    gettimeofday(&start, NULL);
    char **words = read_file(filename, &word_count);
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken to read the file: %f seconds\n", elapsed);
    if (words == NULL) return 1;

    // Allocate a large enough array to store the frequency data
    WordFrequency freq_array[MAX_FREQ_WORDS];
    int freq_count = 0;  // To keep track of unique words in the frequency array

    // Divide the work among threads
    int chunk_size = word_count / num_threads;
    gettimeofday(&start, NULL);
    for (int i = 0; i < num_threads; i++) {
        int start_idx = i * chunk_size;
        int end_idx = (i == num_threads - 1) ? word_count : (i + 1) * chunk_size;

        // Allocate and initialize thread arguments
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->words = words;
        args->start = start_idx;
        args->end = end_idx;
        args->freq_array = freq_array;
        args->freq_count = &freq_count;

        pthread_create(&threads[i], NULL, process_chunk, args);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken for thread creation: %f seconds\n", elapsed);

    // Wait for all threads to complete
    gettimeofday(&start, NULL);
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken for threads to process: %f seconds\n", elapsed);

    // Sort the entire array of words
    qsort(words, word_count, sizeof(char *), compare_words);

    // Count frequencies of sorted words
    freq_count = 0;  // Reset frequency count for final frequency calculation
    for (int i = 0; i < word_count; i++) {
        if (freq_count == 0 || strcmp(words[i], freq_array[freq_count - 1].word) != 0) {
            freq_array[freq_count].word = words[i];
            freq_array[freq_count].frequency = 1;
            freq_count++;
        } else {
            freq_array[freq_count - 1].frequency++;
        }
    }

    // Sort frequency array based on frequency
    qsort(freq_array, freq_count, sizeof(WordFrequency), compare_frequency);

    printf("\nTop 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < freq_count; i++) {
        printf("Word: %s, Frequency: %d\n", freq_array[i].word, freq_array[i].frequency);
    }

    gettimeofday(&end, NULL);
    double total_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("\nTotal time for the entire program: %f seconds\n", total_elapsed);

    free_words(words, word_count);
    return 0;
}
