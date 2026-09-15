#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

// Ключ — long (идентификатор узла), значение — указатель на DSNode.
// Форвард-объявление: полное определение HashTable лежит в lab4/hash_table/generic.h
typedef struct HashTable HashTable;

typedef struct DSNode
{
    long value;
    struct DSNode *parent;
} DSNode;

typedef struct
{
    HashTable *trees; // long -> DSNode*
} DisjointSet;

// Создание узла (MakeSet)
DSNode *make_node(long value);

// Создание DSU
DisjointSet *dsu_create(void);
DisjointSet *dsu_create_from_hash_table(HashTable *nodes);

// FindSet — поиск представителя
DSNode *find_set(DSNode *node);
DSNode *find_set_from_key(DisjointSet *dsu, long key);

// Union — объединение множеств
void union_sets(DSNode *a, DSNode *b);

void freeDisjointSet(DisjointSet *dsu);

#endif // DISJOINT_SET_H