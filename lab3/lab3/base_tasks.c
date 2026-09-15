#include "base_tasks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Student *findMaxStudent(GenericList *list)
{
    if (list == NULL || list->head == NULL)
        return NULL;

    Student *maxStudent = (Student *)list->head->data;
    Node *current = list->head->next;

    while (current != NULL) {
        Student *currentStudent = (Student *)current->data;
        if (currentStudent->avg > maxStudent->avg)
            maxStudent = currentStudent;
        current = current->next;
    }

    return maxStudent;
}

void *findMaxVector(Vector *vector, EqualsFunc cmp)
{
    if (vector == NULL || vector->size == 0)
        return NULL;

    void *maxItem = getVectorItem(vector, 0);

    for (size_t i = 1; i < vector->size; i++) {
        void *current = getVectorItem(vector, i);
        if (cmp(current, maxItem)) {
            maxItem = current;
        }
    }

    void *result = malloc(vector->elem_size);
    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, maxItem, vector->elem_size);
    return result;
}

int removeDuplicatesList(GenericList *list, EqualsFunc cmp)
{
    if (list == NULL || list->head == NULL) {
        return -1;
    }

    Node *current = list->head;
    while (current != NULL) {
        Node *runner = current;
        while (runner->next != NULL) {
            if (cmp(current->data, runner->next->data)) {
                Node *duplicate = runner->next;
                runner->next = duplicate->next;

                free(duplicate->data);
                free(duplicate);
            } else {
                runner = runner->next;
            }
        }
        current = current->next;
    }

    return 0;
}

int removeDuplicatesVector(Vector *vector, EqualsFunc cmp)
{
    if (vector == NULL || vector->size == 0) {
        return -1;
    }

    size_t write_idx = 0;

    for (size_t read_idx = 0; read_idx < vector->size; read_idx++)
    {
        void *read_elem = getVectorItem(vector, read_idx);
        bool is_duplicate = false;

        for (size_t k = 0; k < write_idx; k++)
        {
            void *unique_elem = getVectorItem(vector, k);
            if (cmp(unique_elem, read_elem))
            {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate)
        {
            if (write_idx != read_idx)
            {
                setVectorItem(vector, write_idx, read_elem);
            }
            write_idx++;
        }
    }

    vector->size = write_idx;

    return 0;
}