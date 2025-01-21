#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MATRIX_SIZE 5
#define NO_PARENT (MATRIX_SIZE + 1)
#define NOT_VISITED (MATRIX_SIZE + 2)

typedef struct QueueElement {
    size_t val;
    struct QueueElement* next;
} QueueElement;

typedef struct {
    QueueElement* head;
    QueueElement* tail;
    size_t size;
} Queue;

bool isEmpty(Queue* queue) {
    return queue->size == 0;
}

void enqueue(Queue* queue, QueueElement* element) {
    if (isEmpty(queue)) {
        queue->head = queue->tail = element;
    } else {
        queue->tail->next = element;
        queue->tail = queue->tail->next;
    }
    queue->size++;
}

size_t deque(Queue* queue) {
    size_t val = queue->head->val;
    QueueElement* toFree = queue->head;
    if (queue->size == 1) {
        queue->head = queue->tail = NULL;
    } else {
        queue->head = queue->head->next;
    }
    queue->size--;
    free(toFree);
    return val;
}

void printQueue(Queue* queue) {
    QueueElement* p = queue->head;
    while (p) {
        printf("%zu ", p->val);
        p = p->next;
    }
    printf("\n");
}

QueueElement* createNewElement(size_t value) {
    QueueElement* e = malloc(sizeof(QueueElement));
    e->val = value;
    e->next = NULL;
    return e;
}

void freeQueue(Queue* queue) {
    QueueElement* p = queue->head;
    QueueElement* temp;
    while (p) {
        temp = p;
        p = p->next;
        free(temp);
    }
}

void bfs(const bool** matrix, const size_t start, const size_t end) {
    Queue queue = {.head = NULL, .tail = NULL, .size = 0};
    size_t parent[MATRIX_SIZE];
    for (int i = 0; i < MATRIX_SIZE; i++)
        parent[i] = NOT_VISITED;
    
    parent[start] = NO_PARENT;
    QueueElement* elem = createNewElement(start);
    enqueue(&queue, elem);

    while (!isEmpty(&queue)) {
        size_t cur = deque(&queue);
        /* Check if the destination is found. */
        if (cur == end) {
            while (cur != NO_PARENT) {
                printf("%zu ", cur);
                cur = parent[cur];
            }
            printf("\n");
            freeQueue(&queue);
            return;
        }
        /* Iterate over all neighbours. */
        for (size_t n = 0; n < MATRIX_SIZE; n++) {
            if (matrix[cur][n] && parent[n] == NOT_VISITED) {
                parent[n] = cur;
                QueueElement* e = createNewElement(n);
                enqueue(&queue, e);
            }
        }
    }
    printf("No path found\n");
    return;
}

int main(void) {
    bool row1[MATRIX_SIZE] = {0, 1, 1, 0, 1};
    bool row2[MATRIX_SIZE] = {0, 0, 1, 0, 1};
    bool row3[MATRIX_SIZE] = {1, 1, 0, 1, 1};
    bool row4[MATRIX_SIZE] = {0, 1, 1, 0, 1};
    bool row5[MATRIX_SIZE] = {1, 1, 1, 0, 0};
    bool* matrix[MATRIX_SIZE] = {&row1, &row2, &row3, &row4, &row5};
    bfs(matrix, 0, 3);
    return 0;
}