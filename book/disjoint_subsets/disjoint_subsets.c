#include <stdio.h>

#define VALUE 16
#define PARENT_NOT_DEFINED VALUE + 1
#define ERROR VALUE + 2
#define NEXT printf("\n")

void fill(size_t list[VALUE]) {
    for (int i = 0; i < VALUE; i++)
        list[i] = PARENT_NOT_DEFINED;
}

void print_list(size_t list[VALUE]) {
    printf("[");
    for (int i = 0; i < VALUE - 1; i++)
        printf("%zu, ", list[i]);
    printf("%zu", list[VALUE - 1]);
    printf("]\n");
}

size_t find(size_t value, size_t list[VALUE]) {
    if (value >= VALUE)
        return ERROR;
    if (list[value] == PARENT_NOT_DEFINED)
        return ERROR;
    if (list[value] != value)
        list[value] = find(list[value], list);
    return list[value];
}

size_t* create_list() {
    static size_t list[VALUE];
    fill(list);
    return list;
}

void create_subset(size_t value, size_t list[VALUE]) {
    if (value < VALUE && list[value] == PARENT_NOT_DEFINED)
        list[value] = value;
}

void unite(size_t val1, size_t val2, size_t list[VALUE]) {
    size_t parent1 = find(val1, list);
    size_t parent2 = find(val2, list);

    if (parent1 == ERROR || parent2 == ERROR) return;

    if (parent1 < parent2) {
        list[parent2] = parent1;
    } else {
        list[parent1] = parent2;
    }
}

int main(void) {
    size_t* list = create_list();

    print_list(list); NEXT;

    /* find gives errors */
    printf("%zu, %zu, %zu\n", find(-1, list), find(16, list), find(5, list)); NEXT;

    /* add a wrong and a correct subset */
    create_subset(VALUE, list);
    create_subset(0, list);
    print_list(list); NEXT;

    /* check simple find operation */
    printf("%zu\n", find(0, list)); NEXT;

    /* add new subsets and create a chain 3->1->0 */
    create_subset(1, list);
    create_subset(2, list);
    create_subset(3, list);
    print_list(list);
    unite(1, 3, list);
    print_list(list);
    unite(0, 3, list);
    print_list(list); NEXT;

    /* check that flattening works */
    find(3, list);
    print_list(list); NEXT;
    
    return 0;
}