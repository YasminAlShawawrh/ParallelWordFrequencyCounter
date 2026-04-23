//multiprocessing code
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_WORD_LENGTH 120
#define MAX_WORDS 18000000
#define MAX_FREQ_WORDS 260000

typedef struct
{
    char *word;
    int frequency;
} WordFrequency;

// Function to read the file and store words
char **read_file(char *filename, int *count)
{
    char buffer[MAX_WORD_LENGTH];
    *count = 0;
    char **words = malloc(MAX_WORDS * sizeof(char *));
    if (words == NULL)
    {
        printf("Error: Memory allocation for words array failed.\n");
        return NULL;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("File not found: %s.\n", filename);
        free(words);
        return NULL;
    }

    while (fscanf(file, "%s", buffer) != EOF && *count < MAX_WORDS)
    {
        if (strlen(buffer) > 0 && strlen(buffer) < MAX_WORD_LENGTH)
        {
            words[*count] = malloc(strlen(buffer) + 1);
            if (words[*count] == NULL)
            {
                printf("Error: Memory allocation failed for word %d.\n", *count);
                break;
            }
            strcpy(words[*count], buffer);
            (*count)++;
        }
    }

    fclose(file);
    return words;
}

// Function to compare words for sorting
int compare_words(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void process_chunk(char **words, int start, int end, FILE *temp_file)
{
    WordFrequency freq_array[MAX_FREQ_WORDS];
    int count = 0;
    for (int i = start; i < end; i++)
    {
        if (count == 0 || strcmp(words[i], freq_array[count - 1].word) != 0)
        {
            freq_array[count].word = words[i];
            freq_array[count].frequency = 1;
            count++;
        }
        else
        {
            freq_array[count - 1].frequency++;
        }
    }
    for (int i = 0; i < count; i++)
    {
        fprintf(temp_file, "%s %d\n", freq_array[i].word, freq_array[i].frequency);
    }
}

// Function to compare frequencies of words
int compare_frequency(const void *a, const void *b)
{
    WordFrequency *wordA = (WordFrequency *)a;
    WordFrequency *wordB = (WordFrequency *)b;
    return wordB->frequency - wordA->frequency;
}

// Function to free memory allocated for words
void free_words(char **words, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(words[i]);
    }
    free(words);
}
int main()
{
    struct timeval start, end;
    double elapsed;
    gettimeofday(&start, NULL);
    int num_processes = 8; // Use 2 processes or 4 or 6 or 8
    pid_t pids[num_processes];
    char *filename = "large_file.txt";
    int word_count = 0;
    gettimeofday(&start, NULL);
    char **words = read_file(filename, &word_count);
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken to read the file: %f seconds\n", elapsed);
    if (words == NULL) return 1;
    gettimeofday(&start, NULL);
    qsort(words, word_count, sizeof(char *), compare_words);
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("Time taken to sort the words: %f seconds\n", elapsed);
    int chunk_size = word_count / num_processes;
    for (int i = 0; i < num_processes; i++)
    {
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("Fork failed");
            return 1;
        }
        if (pids[i] == 0)
        {
            char temp_filename[50];
            snprintf(temp_filename, sizeof(temp_filename), "temp_file_%d.txt", i);
            FILE *temp_file = fopen(temp_filename, "w");
            if (temp_file == NULL)
            {
                perror("Error creating temp file");
                exit(1);
            }
            int start_idx = i * chunk_size;
            int end_idx;
            if (i == num_processes - 1)
            {
                end_idx = word_count;
            }
            else
            {
                end_idx = (i + 1) * chunk_size;
            }

            process_chunk(words, start_idx, end_idx, temp_file);
            fclose(temp_file);
            exit(0);
        }
    }
    // Wait for all child processes to finish
    for (int i = 0; i < num_processes; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    // Combine results from temporary files
    FILE *final_file = fopen("final_output.txt", "w");
    if (final_file == NULL)
    {
        perror("Error creating final output file");
        return 1;
    }
    WordFrequency all_freq[MAX_FREQ_WORDS];
    int all_count = 0;
    for (int i = 0; i < num_processes; i++)
    {
        char temp_filename[50];
        snprintf(temp_filename, sizeof(temp_filename), "temp_file_%d.txt", i);

        FILE *temp_file = fopen(temp_filename, "r");
        if (temp_file == NULL)
        {
            perror("Error reading temp file");
            continue;
        }

        char word[MAX_WORD_LENGTH];
        int frequency;
        while (fscanf(temp_file, "%s %d", word, &frequency) != EOF)
        {
            int found = 0;
            for (int j = 0; j < all_count; j++)
            {
                if (strcmp(all_freq[j].word, word) == 0)
                {
                    all_freq[j].frequency += frequency;
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                all_freq[all_count].word = strdup(word);
                all_freq[all_count].frequency = frequency;
                all_count++;
            }
        }
        fclose(temp_file);
        remove(temp_filename);
    }
    qsort(all_freq, all_count, sizeof(WordFrequency), compare_frequency);
    printf("\nTop 10 most frequent words:\n");
    for (int i = 0; i < 10 && i < all_count; i++)
    {
        printf("Word: %s, Frequency: %d\n", all_freq[i].word, all_freq[i].frequency);
    }
    gettimeofday(&end, NULL);
    double total_elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("\nTotal time for the entire program: %f seconds\n", total_elapsed);
    free_words(words, word_count);

    return 0;
}
