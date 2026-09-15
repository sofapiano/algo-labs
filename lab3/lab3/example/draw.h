#include <stdlib.h> 
#include <stdio.h>

struct Point {
    int x;
    int y;
};

struct LinkedList {
    struct Point data;
    struct LinkedList *next;
};


struct Point createPoint(int x, int y) {
    struct Point p;

    p.x = x;
    p.y = y;

    return p;
}

struct LinkedList* newList(struct Point data) {
    struct LinkedList *node = (struct LinkedList*) malloc(sizeof(struct LinkedList));

    if (!node) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    node->data = data;
    node->next = NULL;

    return node;
}

void appendItem(struct LinkedList *head, struct Point data) {
    struct LinkedList *current = head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newList(data);
}

void freeList(struct LinkedList *head) {
    struct LinkedList *current = head;

    while (current != NULL) {
        struct LinkedList *next = current->next;
        free(current);
        current = next;
    }
}

void printList(struct LinkedList *head) {
    struct LinkedList *current = head;

    int i = 1;
    while (current != NULL) {
        printf("Point %d: x=%d, y=%d\n", i++, current->data.x, current->data.y);
        current = current->next;
    }
}
