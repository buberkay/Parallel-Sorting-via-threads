#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void readTextFile(char *filename, char **words, int *num_words) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Dosya açılırken hata oluştu");
        exit(EXIT_FAILURE);
    }
    
    *num_words = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        
        words[*num_words] = strdup(line);
        (*num_words)++;
    }
    fclose(file);
}

void merge(char **words, int left, int middle, int right) {
    int n1 = middle - left + 1;
    int n2 = right - middle;

    char *left_words[n1];
    char *right_words[n2];

    for (int i = 0; i < n1; i++)
        left_words[i] = words[left + i];
    for (int j = 0; j < n2; j++)
        right_words[j] = words[middle + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (strcmp(left_words[i], right_words[j]) <= 0) {
            words[k] = left_words[i];
            i++;
        } else {
            words[k] = right_words[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        words[k] = left_words[i];
        i++;
        k++;
    }
    while (j < n2) {
        words[k] = right_words[j];
        j++;
        k++;
    }
}

void sequentialMergeSort(char **words, int left, int right) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        sequentialMergeSort(words, left, middle);
        sequentialMergeSort(words, middle + 1, right);

        merge(words, left, middle, right);
    }
}

void *parallelMergeSortHelper(void *arg) {
    struct {
        char **words;
        int left;
        int right;
    } *args = arg;

    parallelMergeSort(args->words, args->left, args->right);
    return NULL;
}

void parallelMergeSort(char **words, int left, int right) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        pthread_t thread_left, thread_right;
        struct {
            char **words;
            int left;
            int right;
        } arg_left = {words, left, middle};
        struct {
            char **words;
            int left;
            int right;
        } arg_right = {words, middle + 1, right};

        pthread_create(&thread_left, NULL, parallelMergeSortHelper, &arg_left);
        pthread_create(&thread_right, NULL, parallelMergeSortHelper, &arg_right);

        pthread_join(thread_left, NULL);
        pthread_join(thread_right, NULL);

        merge(words, left, middle, right);
    }
}

int partition(char **words, int low, int high) {
    char *pivot = words[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (strcmp(words[j], pivot) <= 0) {
            i++;
            char *temp = words[i];
            words[i] = words[j];
            words[j] = temp;
        }
    }

    char *temp = words[i + 1];
    words[i + 1] = words[high];
    words[high] = temp;

    return i + 1;
}

void sequentialQuickSort(char **words, int low, int high) {
    if (low < high) {
        int pi = partition(words, low, high);

        sequentialQuickSort(words, low, pi - 1);
        sequentialQuickSort(words, pi + 1, high);
    }
}

void *parallelQuickSortHelper(void *arg) {
    struct {
        char **words;
        int low;
        int high;
    } *args = arg;

    parallelQuickSort(args->words, args->low, args->high);
    return NULL;
}

void parallelQuickSort(char **words, int low, int high) {
    if (low < high) {
        int pi = partition(words, low, high);

        pthread_t thread_left, thread_right;
        struct {
            char **words;
            int low;
            int high;
        } arg_left = {words, low, pi - 1};
        struct {
            char **words;
            int low;
            int high;
        } arg_right = {words, pi + 1, high};

        pthread_create(&thread_left, NULL, parallelQuickSortHelper, &arg_left);
        pthread_create(&thread_right, NULL, parallelQuickSortHelper, &arg_right);

        pthread_join(thread_left, NULL);
        pthread_join(thread_right, NULL);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Kullanım: %s <inputfile> <outputfile> <# of threads> <algorithm>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *inputfile = argv[1];
    char *outputfile = argv[2];
    int num_threads = atoi(argv[3]);
    char *algorithm = argv[4];

    int num_words;
    char *words[1000]; 
    readTextFile(inputfile, words, &num_words);

    if (strcmp(algorithm, "merge") == 0) {
        parallelMergeSort(words, 0, num_words - 1);
        printf("Düzenlenen dosya oluşturuldu: %s\n", outputfile);
    } else if (strcmp(algorithm, "quick") == 0) {
        parallelQuickSort(words, 0, num_words - 1);
        printf("Düzenlenen dosya oluşturuldu: %s\n", outputfile);        
    } else {
        printf("Geçersiz algoritma seçimi\n");
        return EXIT_FAILURE;
    }

    FILE *output = fopen(outputfile, "w");
    if (output == NULL) {
        perror("Çıktı dosyası oluşturulurken hata oluştu");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < num_words; i++) {
        fprintf(output, "%s\n", words[i]);
        free(words[i]); // 
    }
    fclose(output);

    return EXIT_SUCCESS;
}