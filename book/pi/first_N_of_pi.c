#include <stdio.h>
#include <math.h>

// Function to compute arctan(1/x) using the Taylor series
double arctan(double x, int terms) {
    double result = 0.0;
    double term = x;
    int sign = 1;

    for (int i = 1; i <= terms; i += 2) {
        result += sign * term / i;
        term *= (x * x);
        sign = -sign;
    }

    return result;
}

// Function to compute pi using Machin's formula
long double compute_pi(int terms) {
    // Machin's formula: π = 16 * arctan(1/5) - 4 * arctan(1/239)
    long double pi = 16.0 * arctan(1 / 5.0, terms) - 4.0 * arctan(1 / 239.0, terms);
    return pi;
}

int main() {
    int terms;
    M_PI;
    printf("Enter the number of terms for the series: ");
    scanf("%d", &terms);

    long double pi = compute_pi(terms);
    printf("Approximation of π using %d terms: %.100Lf\n", terms, pi);

    return 0;
}
