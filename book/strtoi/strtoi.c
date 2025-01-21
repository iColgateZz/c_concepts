#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CONVERSION_CONST 48

int strtoi(const char* str, size_t n) {
    int val = 0;
    for (int i = 0; i < n; i++) {
        val += (str[i] - CONVERSION_CONST) * pow(10, n - 1 - i);
    }
    return val;
}

int main(void) {
    char* str = "10934";
    int result = strtoi(str, 5);
    printf("%s - %d\n", str, result);
    return EXIT_SUCCESS;
}
