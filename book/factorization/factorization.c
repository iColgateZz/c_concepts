#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

/* O(n ^ 1/2) */
bool isPrime(size_t value) {
    if (value < 2) return false;
    else if (value == 2) return true;
    else if (value % 2 == 0) return false;
    else {
        for (size_t divisor = 3; divisor <= sqrt(value); divisor += 2) {
            if (value % divisor == 0) return false;
        }
        return true;
    }
}

/* 
    Hard to determine the complexity.
    The larger the initial value, the 
    larger are the gaps between two 
    prime numbers.
*/
size_t nextPrime(size_t value) {
    if (value <= 2) return 3;
    if (value % 2 == 0) value++;
    else value += 2;
    while (!isPrime(value)) {
        value += 2;
    }
    return value;
}

/*
    Complexity is proportional to
    the previous function.
*/
size_t smallestDivisor(size_t value) {
    assert(value > 1);
    size_t divisor = 2;
    while (value % divisor != 0) {
        divisor = nextPrime(divisor);
    }
    return divisor;
}

void factorizationHelper(size_t value) {
    if (isPrime(value)) printf("%zu ", value);
    else {
        size_t sd = smallestDivisor(value);
        printf("%zu ", sd);
        factorizationHelper(value / sd);
    }
}

void factorization(size_t value) {
    printf("%zu: ", value);
    factorizationHelper(value);
    printf("\n");
}


int main(void) {

    /* Test isPrime */
    printf("%d", isPrime(0));
    printf("%d", isPrime(1));
    printf("%d", isPrime(2));
    printf("%d", isPrime(3));
    printf("%d", isPrime(4));
    printf("%d", isPrime(17));
    printf("%d\n", isPrime(587));

    /* Test nextPrime */
    printf("%zu ", nextPrime(0));
    printf("%zu ", nextPrime(1));
    printf("%zu ", nextPrime(2));
    printf("%zu ", nextPrime(3));
    printf("%zu ", nextPrime(4));
    printf("%zu ", nextPrime(17));
    printf("%zu ", nextPrime(31));
    printf("%zu\n", nextPrime(561));

    /* Test smallestDivisor */
    printf("%zu\n", smallestDivisor(587));

    /* Test factorization */
    factorization(24);
    factorization(120);
    factorization(5040);
    factorization(123138817420824680);

    return EXIT_SUCCESS;
}