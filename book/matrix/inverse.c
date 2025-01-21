#include <stdio.h>
#include <stdlib.h>

void print_matrix(double** matrix) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

double** create_E() {
    double** E = malloc(2 * sizeof(double*));
    for (int i = 0; i < 2; i++) {
        double* row = malloc(2 * sizeof(double));
        for (int j = 0; j < 2; j++) {
            row[j] = ((i + j) % 2 == 0);
        }
        E[i] = row;
    }
    return E;
}

double** inverse(double** matrix) {
    double** result = create_E();

    for (int i = 0; i < 2; i++) {
        double current = matrix[i][i];
        /* Row division */
        for (int j = 0; j < 2; j++) {
            matrix[i][j] = matrix[i][j] / current;
            result[i][j] = result[i][j] / current;
        }
        /* Subtraction */
        for (int j = 0; j < 2; j++) {
            if (j != i) {
                double multiplicator = -matrix[j][i];
                for (int k = 0; k < 2; k++) {
                    matrix[j][k] += multiplicator * matrix[i][k];
                    result[j][k] += multiplicator * result[i][k];
                }
            }
        }
    }
    
    return result;
}

int main(void) {
    double row1[2] = {1, 3};
    double row2[2] = {2, 7};
    double* matrix[2] = {&row1, &row2};
    double** result = inverse(matrix);
    print_matrix(result);
    return 0;
}