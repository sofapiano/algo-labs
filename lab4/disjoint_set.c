// DSU поверх HashTable из lab4. Ключ — long (id узла), значение — DSNode*.
#include <stdlib.h>
#include <string.h>

#include "disjoint_set.h"
#include "../lab3/lab4/hash_table/generic.h"

// Внутренние хэш/компаратор для ключей типа long.
static int dsuHashLong(const void *key)
{
    long long v = (long long)*(const long *)key;
    v ^= v >> 32;
    return (int)v;
}

static int dsuCmpLong(const void *a, const void *b)
{
    return *(const long *)a == *(const long *)b;
}

// MakeSet: одноэлементное множество {value}
DSNode *make_node(long value)
{
    DSNode *node = (DSNode *)malloc(sizeof(DSNode));
    if (node == NULL)
    {
        return NULL;
    }
    node->value = value;
    node->parent = node; // корень указывает сам на себя
    return node;
}

DisjointSet *dsu_create(void)
{
    DisjointSet *dsu = (DisjointSet *)malloc(sizeof(DisjointSet));
    if (dsu == NULL)
    {
        return NULL;
    }
    dsu->trees = createHashTable(sizeof(long), sizeof(DSNode *));
    if (dsu->trees == NULL)
    {
        free(dsu);
        return NULL;
    }
    return dsu;
}

// Строит DSU: каждый ключ из переданной таблицы становится отдельным множеством
DisjointSet *dsu_create_from_hash_table(HashTable *nodes)
{
    if (nodes == NULL)
    {
        return NULL;
    }
    DisjointSet *dsu = dsu_create();
    if (dsu == NULL)
    {
        return NULL;
    }

    Vector *slots = nodes->values;
    for (size_t i = 0; i < nodes->capacity; i++)
    {
        unsigned char *slot = (unsigned char *)getVectorItem(slots, i);
        if (slot == NULL || *slot != SLOT_OCCUPIED)
        {
            continue;
        }
        long key = 0;
        size_t copy = nodes->key_size < sizeof(long) ? nodes->key_size : sizeof(long);
        memcpy(&key, slot + 1, copy);
        DSNode *node = make_node(key);
        if (node != NULL)
        {
            setItemHashTable(dsu->trees, &key, &node, dsuHashLong, dsuCmpLong);
        }
    }
    return dsu;
}

// FindSet: поднимаемся по родителям до корня
DSNode *find_set(DSNode *node)
{
    if (node == NULL)
    {
        return NULL;
    }
    while (node->parent != node)
    {
        node = node->parent;
    }
    return node;
}

DSNode *find_set_from_key(DisjointSet *dsu, long key)
{
    if (dsu == NULL)
    {
        return NULL;
    }
    DSNode **stored = (DSNode **)getItemHashTable(dsu->trees, &key, dsuHashLong, dsuCmpLong);
    if (stored == NULL)
    {
        return NULL;
    }
    return *stored;
}

// Union: присоединяем корень второго к корню первого
void union_sets(DSNode *a, DSNode *b)
{
    if (a == NULL || b == NULL)
    {
        return;
    }
    DSNode *root_a = find_set(a);
    DSNode *root_b = find_set(b);
    if (root_a != root_b)
    {
        root_b->parent = root_a;
    }
}

void freeDisjointSet(DisjointSet *dsu)
{
    if (dsu == NULL)
    {
        return;
    }
    if (dsu->trees != NULL)
    {
        // сначала освобождаем сами DSNode (они лежат по указателям в таблице)
        Vector *slots = dsu->trees->values;
        for (size_t i = 0; i < dsu->trees->capacity; i++)
        {
            unsigned char *slot = (unsigned char *)getVectorItem(slots, i);
            if (slot != NULL && *slot == SLOT_OCCUPIED)
            {
                DSNode *node = NULL;
                memcpy(&node, slot + 1 + dsu->trees->key_size, sizeof(node));
                free(node);
            }
        }
        freeHashTable(dsu->trees);
    }
    free(dsu);
}