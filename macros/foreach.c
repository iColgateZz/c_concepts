#include <stdio.h>

#define foreach(ty, array) \
    for (struct { \
            int index; \
            ty *item;  \
            int count; \
        } _ = {0, NULL, sizeof(array) / sizeof(*array)}; \
         _.index < _.count && (_.item = &array[_.index]) != NULL; _.index++)

struct person {
    char *name;
} person = {"lol"};

int main(void) {
    struct person people[2] = {{"bob"}, {"alice"}};

    printf("%s\n", person.name);
    
    foreach(struct person, people)
        printf("Name: %s\n", _.item->name);
    
    return 0;
}