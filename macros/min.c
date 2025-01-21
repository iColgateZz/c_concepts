#include <stdio.h>

#define min(x, y) (x > y) ? y : x

int main(void) {
    printf("%d\n", min(3, 5));
    return 0;
}