#include <stdio.h>
#include <stdbool.h>

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
        printf("Queue is empty\n");
        queue->head = queue->tail = element;
    } else {
        printf("Enque multiple\n");
        queue->tail->next = element;
        queue->tail = queue->tail->next;
    }
    queue->size++;
}

size_t deque(Queue* queue) {
    size_t val = queue->head->val;
    if (queue->size == 1) {
        printf("Deque 1\n");
        queue->head = queue->tail = NULL;
    } else {
        printf("Deque multiple\n");
        queue->head = queue->head->next;
    }
    queue->size--;
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

int main(void) {
    Queue queue = {.head = NULL, .tail = NULL, .size = 0};
    QueueElement e = {.val = 2, .next = NULL};
    // enqueue(&queue, &e);
    // e.val = 3;
    // enqueue(&queue, &e);
    // e.val = 5;
    // enqueue(&queue, &e);
    // // deque(&queue);
    // // deque(&queue);
    // // deque(&queue);
    // printQueue(&queue);
    printf("%lu\n", sizeof(QueueElement));
    printf("%lu\n", sizeof(size_t));
    return 0;
}