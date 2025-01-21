/* text_proc.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1024

/* structures */

typedef struct DLListNode {
    char str[MAX_SIZE];
    struct DLListNode* next;
    struct DLListNode* prev;
} DLListNode;

typedef struct DLList {
    struct DLListNode* head;
    struct DLListNode* tail;
} DLList;


/* functions */

DLListNode* createNode(char buf[MAX_SIZE]) {
    DLListNode* node = malloc(sizeof(DLListNode));
    strcpy(node->str, buf);
    node->next = NULL;
    node->prev = NULL;
    return node;
}

void linkNode(DLList* list, char buf[MAX_SIZE]) {
    DLListNode* node = createNode(buf);
    if (list->tail) {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = list->tail = node;
    }
}

void printList(DLList* list) {
    DLListNode* p;
    int i;

    p = list->head;
    i = 0;
    printf("\n");
    while (p) {
        printf("%d: %s\n", i, p->str);
        p = p->next;
        i++;
    }
}

void merge(DLList* list, int first, int second) {
    if (second - first != 1) {
        printf("Sentences must be consecutive\n");
        return;
    }

    int i = 0;
    DLListNode* p1 = list->head;
    DLListNode* p2;
    DLListNode* p3;
    while (p1) {
        if (i == first) break;
        p1 = p1->next;
        i++;
    }
    p2 = p1->next;
    p3 = p2->next;

    int n = strlen(p1->str);
    int m = strlen(p2->str);

    if (n + m >= MAX_SIZE - 1) {
        printf("Sentences are too large to merge\n");
        return;
    }

    p1->next = p3;
    if (p3) {
        p3->prev = p1;
    }

    p1->str[n] = ' ';
    strcpy(p1->str + n + 1, p2->str);
    free(p2);
    return;
}

void separate(DLList* list, int first, int second) {
    int i;
    DLListNode* p1;
    DLListNode* p2;
    DLListNode* p3;
    char buf[MAX_SIZE];

    i = 0;
    p1 = list->head;
    while (p1) {
        if (i == first) break;
        p1 = p1->next;
        i++;
    }
    p2 = p1->next;

    int n = strlen(p1->str);
    if (second >= n) {
        printf("Incorrent separation spot\n");
        return;
    }

    strcpy(buf, p1->str + second);
    printf("Buffer: %s\n", buf);
    p1->str[second] = '\0';
    printf("Modified p1: %s\n", p1->str);
    p3 = createNode(buf);
    printf("p3 str: %s\n", p3->str);
    p3->prev = p1;
    p3->next = p2;

    p1->next = p3;
    if (p2) {
        p2->prev = p3;
    }

    return;
}


int main(void) {
    char* text;
    DLList list;
    char buf[MAX_SIZE];
    int k, action, first, second;

    text = 
    "What are you doing?\n"
    "I asked you not to do that.\n"
    "Why can't you just behave for once?!?\n"
    "That is insane. You are driving me crazy.\n";

    memset(&list, 0, sizeof(DLList));
    memset(&buf, 0, MAX_SIZE);
    list.head = NULL;
    list.tail = NULL;

    k = 0;
    for (int i = 0; i < strlen(text); i++) {
        if (text[i] == '\n') {
            buf[k] = 0;
            k = 0;
            linkNode(&list, buf);
            memset(&buf, 0, MAX_SIZE);
        } else {
            buf[k] = text[i];
            k++;
        }
    }

    while (1) {
        printList(&list);

        printf("\nChoose an action (0 - merge, 1 - separate): ");
        scanf("%d", &action);
        printf("First parameter: ");
        scanf("%d", &first);
        printf("Second parameter: ");
        scanf("%d", &second);

        if (action) {
            separate(&list, first, second);
        } else {
            merge(&list, first, second);
        }
    }

    return 0;
}