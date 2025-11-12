// Written by macphi

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int* a;
int n;

int nb_threads(int taille_tableau) {
    if (taille_tableau <= 2) return 1;
    return (taille_tableau - 1) / 2;
}

typedef struct {
    int phase;
    int thread_id;
    int num_threads;
    int *swapped;
} thread_data_t;

void compare_swap(int *a, int *b) {
    if (*a > *b) {
        int temp = *a;
        *a = *b;
        *b = temp;
    }
}

void* phase_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    int phase = data->phase;
    int thread_id = data->thread_id;
    int num_threads = data->num_threads;
    if (phase == 0) { // Indices pairs
        int start = thread_id * 2;
        for (int j = start; j < n - 1; j += num_threads * 2) {
            if (a[j] > a[j + 1]) {
                compare_swap(&a[j], &a[j + 1]);
                *(data->swapped) = 1;
            }
        }
    } else { // Indices impairs
        int start = thread_id * 2 + 1;
        for (int j = start; j < n - 1; j += num_threads * 2) {
            if (a[j] > a[j + 1]) {
                compare_swap(&a[j], &a[j + 1]);
                *(data->swapped) = 1;
            }
        }
    }
    return NULL;
}

void tri_bubulle(int* a_arr, int n_arr) {
    a = a_arr;
    n = n_arr;
    int num_threads = nb_threads(n);
    printf("Tableau taille %d -> Utilisation de %d thread%s\n", n, num_threads, num_threads > 1 ? "s" : "");

    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    int swapped = 1;
    int iteration = 0;
    while (swapped && iteration < n) {
        swapped = 0;
        // Phase paire
        for (int i = 0; i < num_threads; i++) {
            thread_data[i].phase = 0;
            thread_data[i].thread_id = i;
            thread_data[i].num_threads = num_threads;
            thread_data[i].swapped = &swapped;
            pthread_create(&threads[i], NULL, phase_thread, &thread_data[i]);
        }
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
        // Phase impaire
        for (int i = 0; i < num_threads; i++) {
            thread_data[i].phase = 1;
            thread_data[i].thread_id = i;
            thread_data[i].num_threads = num_threads;
            thread_data[i].swapped = &swapped;
            pthread_create(&threads[i], NULL, phase_thread, &thread_data[i]);
        }
        for (int i = 0; i < num_threads; i++) pthread_join(threads[i], NULL);
        iteration++;
    }
    printf("Tri terminé en %d itérations\n", iteration);
}

void print_array(int a[], int n) {
    for (int i = 0; i < n; i++) printf("%d ", a[i]);
    printf("\n");
}

int is_sorted(int a[], int n) {
    for (int i = 0; i < n - 1; i++) if (a[i] > a[i + 1]) return 0;
    return 1;
}

void debug_is_sorted(int a[], int n) {
    if (is_sorted(a, n)) {
        printf("Le tableau est trié.\n");
    } else {
        printf("Le tableau n'est pas trié.\n");
    }
}

void test_array(int a[], int n) {
    printf("Avant: "); print_array(a, n);
    tri_bubulle(a, n);
    printf("Après: "); print_array(a, n);
    debug_is_sorted(a, n);
    printf("\n");
}

int* generate_random_array(int n, int max_value) {
    int* arr = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (max_value + 1);
    }
    return arr;
}

void test_random_array(int n, int max_value) {
    int* arr = generate_random_array(n, max_value);
    printf("Tableau aléatoire de taille %d:\n", n);
    test_array(arr, n);
    free(arr);
}


int bubulles_sort_file(char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Cannot open file");
        return -2;
    }
    int file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char* file_content = calloc(file_size, sizeof(char));
    if (!file_content) {
        perror("Cannot allocate file_content");
        return -2;
    }

    read(fd, file_content, file_size);

    int spaces = 0;
    for(int i = 0; i < file_size; i++) {
        if (file_content[i] == ' ' || file_content[i] == '\n') spaces++;
        else if (file_content[i] < 0x30 || file_content[i] > 0x39) {
            printf("The file doesn't have the right format\nIt need to be numbers, separted by spaces/newlines !\n");
            return -2;
        }
    }

    int* array = calloc(spaces+1, sizeof(int));
    if (!array) {
        perror("Cannot allocate array");
        return -2;
    }

    int n_numbers = 0;

    char* start_ptr = file_content;
    char* end_ptr;

    do {
        array[n_numbers++] = strtol(start_ptr, &end_ptr, 10);
        start_ptr = end_ptr+1;
    } while (start_ptr != end_ptr && end_ptr < file_content + file_size);

    printf("Before : ");
    print_array(array, n_numbers);
    tri_bubulle(array, n_numbers);
    printf("After : ");
    print_array(array, n_numbers);
    

    return 0;
}

int bubulles_test() {
    // Tests avec différentes tailles
    printf("----- Tri bubulles -----\n\n");
    test_random_array(1,100);
    test_random_array(2,100);
    test_random_array(3,100);
    test_random_array(4,100);
    test_random_array(5,100);
    test_random_array(200,500);

    return 0;
}