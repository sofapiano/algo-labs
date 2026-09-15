#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../../lab3/vector/generic.h"
#include "generic.h"

int HashInt(const void *key)
{
    return *(int *)key;
}

int HashString(const void *key)
{
    unsigned char *str = (unsigned char *)key;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return (int)hash;
}

HashTable *createHashTable(size_t key_size, size_t val_size)
{
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    if (table == NULL)
    {
        return NULL;
    }

    size_t slot_size = 1 + key_size + val_size;
    table->values = createVector(slot_size);
    if (table->values == NULL)
    {
        free(table);
        return NULL;
    }

    table->key_size = key_size;
    table->val_size = val_size;
    table->size = 0;
    table->capacity = TABLE_MIN_SIZE;

    unsigned char empty_slot_bytes[slot_size];
    memset(empty_slot_bytes, SLOT_EMPTY, slot_size);

    for (size_t i = 0; i < TABLE_MIN_SIZE; i++)
    {
        if (appendVectorItem(table->values, empty_slot_bytes) != 0)
        {
            vectorFree(table->values);
            free(table);
            return NULL;
        }
    }

    return table;
}

void rehashHashTable(HashTable *table, HashFunc hash, CmpFunc cmp)
{
    Vector *old_values = table->values;
    size_t old_capacity = table->capacity;
    size_t new_capacity = old_capacity * 2;
    size_t slot_size = 1 + table->key_size + table->val_size;

    Vector *new_values = createVector(slot_size);
    if (new_values == NULL)
    {
        return;
    }

    unsigned char empty_slot_bytes[slot_size];
    memset(empty_slot_bytes, SLOT_EMPTY, slot_size);
    for (size_t i = 0; i < new_capacity; i++)
    {
        if (appendVectorItem(new_values, empty_slot_bytes) != 0)
        {
            vectorFree(new_values);
            return;
        }
    }

    table->values = new_values;
    table->capacity = new_capacity;
    table->size = 0;

    for (size_t i = 0; i < old_capacity; i++)
    {
        unsigned char *slot = (unsigned char *)getVectorItem(old_values, i);
        if (*(unsigned char *)slot == SLOT_OCCUPIED)
        {
            setItemHashTable(table, slot + 1, slot + 1 + table->key_size, hash, cmp);
        }
    }

    vectorFree(old_values);
}

void setItemHashTable(HashTable *table, void *key, void *data, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL || data == NULL)
    {
        return;
    }

    if (table->size >= table->capacity / 2)
    {
        rehashHashTable(table, hash, cmp);
    }

    size_t capacity = table->capacity;
    unsigned int start = (unsigned int)hash(key) % capacity;

    size_t insert_index = 0;
    int found_place = 0;

    for (size_t i = 0; i < capacity; i++)
    {
        size_t index = (start + i * i) % capacity;
        unsigned char *slot = (unsigned char *)getVectorItem(table->values, index);
        unsigned char flag = *(unsigned char *)slot;
        if (flag == SLOT_OCCUPIED)
        {
            if (cmp(slot + 1, key))
            {
                memcpy(slot + 1 + table->key_size, data, table->val_size);
                return;
            }
        }
        else
        {
            if (!found_place)
            {
                insert_index = index;
                found_place = 1;
            }
            if (flag == SLOT_EMPTY)
            {
                break;
            }
        }
    }

    unsigned char *target = (unsigned char *)getVectorItem(table->values, insert_index);
    memset(target, SLOT_OCCUPIED, 1);
    memcpy(target + 1, key, table->key_size);
    memcpy(target + 1 + table->key_size, data, table->val_size);
    table->size++;
}

void *getItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL || hash == NULL || cmp == NULL)
    {
        return NULL;
    }

    size_t capacity = table->capacity;
    unsigned int start = (unsigned int)hash(key) % capacity;

    for (size_t i = 0; i < capacity; i++)
    {
        size_t index = (start + i * i) % capacity;
        unsigned char *slot = (unsigned char *)getVectorItem(table->values, index);
        unsigned char flag = *(unsigned char *)slot;

        if (flag == SLOT_OCCUPIED)
        {
            if (cmp(slot + 1, key))
            {
                return slot + 1 + table->key_size;
            }
        }
        else if (flag == SLOT_EMPTY)
        {
            break;
        }
    }

    return NULL;
}

void *popItemHashTable(HashTable *table, void *key, HashFunc hash, CmpFunc cmp)
{
    if (table == NULL || key == NULL || hash == NULL || cmp == NULL)
    {
        return NULL;
    }

    size_t capacity = table->capacity;
    unsigned int start = (unsigned int)hash(key) % capacity;

    for (size_t i = 0; i < capacity; i++)
    {
        size_t index = (start + i * i) % capacity;
        unsigned char *slot = (unsigned char *)getVectorItem(table->values, index);
        unsigned char flag = *(unsigned char *)slot;

        if (flag == SLOT_OCCUPIED)
        {
            if (cmp(slot + 1, key))
            {
                void *result = malloc(table->val_size);
                if (result == NULL)
                {
                    return NULL;
                }
                memcpy(result, slot + 1 + table->key_size, table->val_size);
                *(unsigned char *)slot = SLOT_DELETED;
                table->size--;
                return result;
            }
        }
        else if (flag == SLOT_EMPTY)
        {
            break;
        }
    }

    return NULL;
}

unsigned long int getCollisionCount(HashTable *table, HashFunc hash)
{
    if (table == NULL || hash == NULL)
    {
        return 0;
    }

    unsigned long int count = 0;
    size_t capacity = table->capacity;

    for (size_t i = 0; i < capacity; i++)
    {
        unsigned char *slot = (unsigned char *)getVectorItem(table->values, i);
        if (*(unsigned char *)slot == SLOT_OCCUPIED)
        {
            size_t primary = (unsigned int)hash(slot + 1) % capacity;
            if (primary != i)
            {
                count++;
            }
        }
    }

    return count;
}

void freeHashTable(HashTable *table)
{
    if (table == NULL)
    {
        return;
    }

    vectorFree(table->values);
    free(table);
}