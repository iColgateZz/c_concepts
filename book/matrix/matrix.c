#include <stdio.h>
#include <stdlib.h>

// void sizeof_arr_from_a_func(int arr[]) {
//     printf("%lu\n", sizeof(arr));
// }

/* Perform the multiplication of 2 vectors of the same size. */
int vector_multiplication(int a[], int b[], size_t size) {
    int val = 0;
    for (int i = 0; i < size; i++)
        val += a[i] * b[i];
    return val;
}

int** matrix_multiplication(int a[][3], int b[][3], size_t n, size_t new_n, size_t new_m) {
    int** arr = malloc(new_m * sizeof(int *));
    for (int i = 0; i < new_m; i++) {
        int* inner = malloc(new_n * sizeof(int));
        for (int j = 0; j < new_n; j++) {
            inner[j] = vector_multiplication(a[j], b[i], n);
        }
        arr[i] = inner;
    }
    return arr;
}

void print_matrix(int** matrix, size_t n, size_t m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

int main(void) {

    int arr[] = {1, 2, 3, 4, 5};
    int matrix1[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    int matrix2[1][3] = {
        {1, 0, -1}
    };
    int** result;

    // printf("%lu\n", sizeof(matrix1));

    /*  If passed to a function, an array decays to a pointer
        thus resulting in sizeof being useless.  */
    // sizeof_arr_from_a_func(arr);
    // printf("%lu\n", sizeof(arr));
    // printf("%lu\n", sizeof(int));

    // printf("%d\n", vector_multiplication(arr, arr, sizeof(arr) / sizeof(arr[0])));
    result = matrix_multiplication(matrix1, matrix2, 3, 3, 1);
    print_matrix(result, 1, 3);

    return 0;
}