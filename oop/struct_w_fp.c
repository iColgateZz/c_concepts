#include <stdio.h>

typedef struct Cat {
    char name[16];
    void (*fp)();
} Cat;

void meow() {
    printf("Meow\n");
    return;
}

int main(void) {
    Cat cat = {"Cat", meow};
    cat.fp();
    return 0;
}