#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "disjoint_set.h"
#include "../lab3/lab4/hash_table/generic.h"

// Хэш/компаратор для тестовой таблицы ключей типа long
static int keyHash(const void *key)
{
    return (int)*(const long *)key;
}

static int keyCmp(const void *a, const void *b)
{
    return *(const long *)a == *(const long *)b;
}

// 1. make_node — узел создан, parent == node
static void test_make_node(void)
{
    DSNode *node = make_node(42);
    assert(node != NULL);
    assert(node->value == 42);
    assert(node->parent == node);
    free(node);
    printf("  make_node: OK\n");
}

// 2. dsu_create — пустая структура создана корректно
static void test_dsu_create(void)
{
    DisjointSet *dsu = dsu_create();
    assert(dsu != NULL);
    assert(dsu->trees != NULL);
    assert(dsu->trees->size == 0);
    freeDisjointSet(dsu);
    printf("  dsu_create: OK\n");
}

// 3. dsu_create_from_hash_table — каждый ключ доступен через find_set_from_key
static void test_dsu_create_from_hash_table(void)
{
    long keys[] = {10, 20, 30, 40, 50, 60};
    int vals[] = {1, 2, 3, 4, 5, 6};
    HashTable *nodes = createHashTable(sizeof(long), sizeof(int));
    assert(nodes != NULL);

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        setItemHashTable(nodes, &keys[i], &vals[i], keyHash, keyCmp);
    }

    DisjointSet *dsu = dsu_create_from_hash_table(nodes);
    assert(dsu != NULL);

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        DSNode *node = find_set_from_key(dsu, keys[i]);
        assert(node != NULL);
        assert(node->value == keys[i]);
        assert(find_set(node) == node); // пока что всё одноэлементное
    }

    // несуществующего ключа нет
    assert(find_set_from_key(dsu, 999) == NULL);

    freeDisjointSet(dsu);
    freeHashTable(nodes);
    printf("  dsu_create_from_hash_table: OK\n");
}

// 4. find_set — одиночный узел сам себе представитель
static void test_find_set_single(void)
{
    DSNode *a = make_node(7);
    assert(find_set(a) == a);
    free(a);
    printf("  find_set (одиночный узел): OK\n");
}

// 5. union_sets — после объединения find_set(a) == find_set(b)
static void test_union_sets(void)
{
    DSNode *a = make_node(1);
    DSNode *b = make_node(2);
    union_sets(a, b);
    assert(find_set(a) == find_set(b));
    free(a);
    free(b);
    printf("  union_sets: OK\n");
}

// 6. Транзитивность: union(a,b), union(b,c) -> find_set(a) == find_set(c)
static void test_transitivity(void)
{
    DSNode *a = make_node(1);
    DSNode *b = make_node(2);
    DSNode *c = make_node(3);
    union_sets(a, b);
    union_sets(b, c);
    assert(find_set(a) == find_set(b));
    assert(find_set(a) == find_set(c));
    assert(find_set(c) == find_set(a));
    free(a);
    free(b);
    free(c);
    printf("  транзитивность: OK\n");
}

// 7. Идемпотентность: повторный union не ломает структуру
static void test_idempotency(void)
{
    DSNode *a = make_node(10);
    DSNode *b = make_node(20);
    union_sets(a, b);
    union_sets(a, b);
    union_sets(a, b); // повторные объединения — как из README
    assert(find_set(a) == find_set(b));
    free(a);
    free(b);
    printf("  идемпотентность: OK\n");
}

// Пример из README:
// Union(1,2), Union(3,4), Union(2,3) ->
// FindSet(4) == FindSet(1): ДА;  FindSet(4) == FindSet(5): НЕТ
static void test_readme_example(void)
{
    DSNode *nodes[5];
    for (long i = 1; i <= 5; i++)
    {
        nodes[i - 1] = make_node(i);
    }
    union_sets(nodes[0], nodes[1]); // Union(1,2)
    union_sets(nodes[2], nodes[3]); // Union(3,4)
    union_sets(nodes[1], nodes[2]); // Union(2,3)
    assert(find_set(nodes[3]) == find_set(nodes[0]));
    assert(find_set(nodes[3]) != find_set(nodes[4]));
    for (size_t i = 0; i < 5; i++)
    {
        free(nodes[i]);
    }
    printf("  пример из README: OK\n");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== Тесты DSU ===\n");

    test_make_node();
    test_dsu_create();
    test_dsu_create_from_hash_table();
    test_find_set_single();
    test_union_sets();
    test_transitivity();
    test_idempotency();
    test_readme_example();

    printf("Все тесты пройдены!\n");
    return 0;
}