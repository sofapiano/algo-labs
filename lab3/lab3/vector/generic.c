#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generic.h"

static bool needToResize(Vector *vector, bool *increase)
{
    if (vector->size >= vector->capacity)
    {
        *increase = true;
        return true;
    }
    if (vector->size < vector->capacity / 4 && vector->capacity > MIN_SIZE)
    {
        *increase = false;
        return true;
    }
    return false;
}

static int resize(Vector *vector, bool increase)
{
    size_t newCapacity = increase ? vector->capacity * 2 : vector->capacity / 2;
    if (newCapacity < MIN_SIZE)
    {
        newCapacity = MIN_SIZE;
    }
    void *newData = realloc(vector->data, newCapacity * vector->elem_size);
    if (newData == NULL)
    {
        return 1;
    }
    vector->data = newData;
    vector->capacity = newCapacity;
    return 0;
}

Vector *createVector(size_t elem_size)
{
    Vector *vector = (Vector *)malloc(sizeof(Vector));
    if (vector == NULL)
    {
        return NULL;
    }
    vector->elem_size = elem_size;
    vector->size = 0;
    vector->capacity = MIN_SIZE;
    vector->data = malloc(vector->capacity * elem_size);
    if (vector->data == NULL)
    {
        free(vector);
        return NULL;
    }
    return vector;
}

int appendVectorItem(Vector *vector, void *el)
{
    bool increase = false;
    if (vector == NULL || el == NULL)
    {
        return 1;
    }
    if (needToResize(vector, &increase) && resize(vector, increase) != 0)
    {
        return 1;
    }
    memcpy((char *)vector->data + vector->size * vector->elem_size, el, vector->elem_size);
    vector->size++;
    return 0;
}

void *getVectorItem(Vector *vector, size_t index)
{
    if (vector == NULL || index >= vector->size)
    {
        return NULL;
    }
    return (char *)vector->data + index * vector->elem_size;
}

int setVectorItem(Vector *vector, size_t index, void *value)
{
    if (vector == NULL || value == NULL || index >= vector->size)
    {
        return 1;
    }
    memcpy((char *)vector->data + index * vector->elem_size, value, vector->elem_size);
    return 0;
}

void *popVectorItem(Vector *vector, size_t index)
{
    if (vector == NULL || index >= vector->size)
    {
        return NULL;
    }
    void *copy = malloc(vector->elem_size);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, (char *)vector->data + index * vector->elem_size, vector->elem_size);
    if (index < vector->size - 1)
    {
        memmove((char *)vector->data + index * vector->elem_size,
                (char *)vector->data + (index + 1) * vector->elem_size,
                (vector->size - index - 1) * vector->elem_size);
    }
    vector->size--;
    bool increase = false;
    if (needToResize(vector, &increase) && resize(vector, increase) != 0)
    {
        // память не смогли сжать, но элемент уже удалён
    }
    return copy;
}

long int findVectorItem(Vector *vector, void *value, EqualsFunc cmp)
{
    if (vector == NULL || value == NULL || cmp == NULL)
    {
        return -1;
    }
    for (size_t i = 0; i < vector->size; i++)
    {
        if (cmp((char *)vector->data + i * vector->elem_size, value))
        {
            return (long int)i;
        }
    }
    return -1;
}

int vectorFree(Vector *vector)
{
    if (vector == NULL)
    {
        return 1;
    }
    free(vector->data);
    free(vector);
    return 0;
}