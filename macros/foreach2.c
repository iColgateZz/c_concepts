#include <stdio.h>

#define foreach(array, _) \
    for (int i = 0, count = sizeof(array) / sizeof(*array); \
        i < count && (_ = &array[i]) != NULL; i++)

struct person {
    char *name;
};

int main(void) {
    struct person people[2] = {{"bob"}, {"alice"}};

    struct person *val;
    foreach(people, val) {
        printf("%s\n", val->name);
    }
    
    return 0;
}