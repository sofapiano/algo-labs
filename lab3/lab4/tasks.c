#include "tasks.h"
#include "hash_table/generic.h"
#include "../lab3/vector/generic.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int HashStringPtr(const void *key)
{
    const char *str = *(const char **)key;
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c;
    return (int)hash;
}

int CmpStringPtr(const void *a, const void *b)
{
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b) == 0;
}

Vector *removeDuplicates(Vector *strings)
{
    if (!strings)
        return NULL;

    Vector *result = createVector(sizeof(char *));
    if (!result)
        return NULL;

    HashTable *seen = createHashTable(sizeof(char *), sizeof(int));
    if (!seen)
    {
        vectorFree(result);
        return NULL;
    }

    for (size_t i = 0; i < strings->size; i++)
    {
        char *str = *(char **)getVectorItem(strings, i);

        if (!getItemHashTable(seen, &str, HashStringPtr, CmpStringPtr))
        {
            int one = 1;
            setItemHashTable(seen, &str, &one, HashStringPtr, CmpStringPtr);
            appendVectorItem(result, &str);
        }
    }

    freeHashTable(seen);
    return result;
}

Vector *encodeStrings(Vector *strings)
{
    if (!strings)
        return NULL;

    Vector *result = createVector(sizeof(int));
    if (!result)
        return NULL;

    HashTable *mapping = createHashTable(sizeof(char *), sizeof(int));
    if (!mapping)
    {
        vectorFree(result);
        return NULL;
    }

    int next_code = 0;

    for (size_t i = 0; i < strings->size; i++)
    {
        char *str = *(char **)getVectorItem(strings, i);
        int *existing = getItemHashTable(mapping, &str, HashStringPtr, CmpStringPtr);

        int code;
        if (existing)
        {
            code = *existing;
        }
        else
        {
            code = next_code++;
            setItemHashTable(mapping, &str, &code, HashStringPtr, CmpStringPtr);
        }

        appendVectorItem(result, &code);
    }

    freeHashTable(mapping);
    return result;
}

HashTable *swapKeysValues(HashTable *table, HashFunc hash, CmpFunc cmp)
{
    if (!table || !hash || !cmp)
        return NULL;

    HashTable *swapped = createHashTable(table->val_size, table->key_size);
    if (!swapped)
        return NULL;

    size_t slot_size = 1 + table->key_size + table->val_size;

    for (size_t i = 0; i < table->capacity; i++)
    {
        int8_t *slot = (int8_t *)table->values->data + i * slot_size;

        if (*(int8_t *)slot == SLOT_OCCUPIED)
        {
            void *old_key = slot + 1;
            void *old_val = slot + 1 + table->key_size;
            setItemHashTable(swapped, old_val, old_key, hash, cmp);
        }
    }

    return swapped;
}
