#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GenericList *createList(size_t elem_size)
{
    GenericList *list = (GenericList *)malloc(sizeof(GenericList));
    if (!list) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    list->head = NULL;
    list->elem_size = elem_size;
    return list;
}

void appendItem(GenericList *list, void *data)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    newNode->data = malloc(list->elem_size);
    if (!newNode->data) {
        free(newNode);
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    memcpy(newNode->data, data, list->elem_size);
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
        return;
    }

    Node *current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

int findItem(GenericList *list, void *value, EqualsFunc cmp)
{
    if (list->head == NULL) {
        return -1;
    }

    Node *current = list->head;
    int index = 0;

    while (current != NULL) {
        if (cmp(current->data, value) == 1) {
            return index;
        }
        current = current->next;
        index++;
    }

    return -1;
}

void *popItem(GenericList *list, size_t index)
{
    if (list->head == NULL) {
        return NULL;
    }

    if (index == 0) {
        Node *toDelete = list->head;
        void *copy = malloc(list->elem_size);
        if (!copy) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(EXIT_FAILURE);
        }
        memcpy(copy, toDelete->data, list->elem_size);
        list->head = toDelete->next;
        free(toDelete->data);
        free(toDelete);
        return copy;
    }

    Node *prev = list->head;
    for (size_t i = 0; i < index - 1; i++) {
        if (prev->next == NULL) {
            return NULL;
        }
        prev = prev->next;
    }

    if (prev->next == NULL) {
        return NULL;
    }

    Node *toDelete = prev->next;
    void *copy = malloc(list->elem_size);
    if (!copy) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(copy, toDelete->data, list->elem_size);
    prev->next = toDelete->next;
    free(toDelete->data);
    free(toDelete);
    return copy;
}

void freeList(GenericList *list)
{
    if (list == NULL) {
        return;
    }

    Node *current = list->head;

    while (current != NULL) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }

    free(list);
}

unsigned int listLength(GenericList *list)
{
    if (list == NULL || list->head == NULL) {
        return 0;
    }

    unsigned int count = 0;
    Node *current = list->head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}
