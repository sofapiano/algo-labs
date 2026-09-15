#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(condition, message)                                             \
    do {                                                                      \
        if (condition) {                                                      \
            tests_passed++;                                                   \
            printf("  [PASS] %s\n", message);                                 \
        } else {                                                              \
            tests_failed++;                                                   \
            printf("  [FAIL] %s\n", message);                                 \
        }                                                                     \
    } while (0)

static void test_empty_table(void)
{
    printf("Test: пустая таблица\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    int key = 5;
    CHECK(getItemHashTable(t, &key, HashInt, intEquals) == NULL,
          "getItemHashTable в пустой таблице возвращает NULL");
    CHECK(popItemHashTable(t, &key, HashInt, intEquals) == NULL,
          "popItemHashTable в пустой таблице возвращает NULL");
    CHECK(getCollisionCount(t, HashInt) == 0,
          "getCollisionCount пустой таблицы равен 0");
    CHECK(t->size == 0 && t->capacity == TABLE_MIN_SIZE,
          "size == 0, capacity == TABLE_MIN_SIZE");

    freeHashTable(t);
}

static void test_minimal_data(void)
{
    printf("Test: минимальные данные (один элемент)\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    int key = 10, val = 100;
    setItemHashTable(t, &key, &val, HashInt, intEquals);

    int *res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL, "getItemHashTable находит элемент");
    CHECK(res != NULL && *res == 100, "значение равно 100");
    CHECK(t->size == 1, "size == 1 после вставки");

    int missing = 99;
    CHECK(getItemHashTable(t, &missing, HashInt, intEquals) == NULL,
          "несуществующий ключ возвращает NULL");

    freeHashTable(t);
}

static void test_collisions(void)
{
    printf("Test: коллизии (ключи 1, 11, 21)\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    /* 1, 11 и 21 при capacity = 10 дают один и тот же primary-слот 1 */
    int keys[3] = {1, 11, 21};
    int vals[3] = {100, 110, 210};
    for (int i = 0; i < 3; i++)
    {
        setItemHashTable(t, &keys[i], &vals[i], HashInt, intEquals);
    }

    CHECK(t->size == 3, "все три ключа вставлены");
    CHECK(getCollisionCount(t, HashInt) == 2,
          "элементы 11 и 21 лежат не в своём primary-слоте (2 коллизии)");

    for (int i = 0; i < 3; i++)
    {
        int *res = getItemHashTable(t, &keys[i], HashInt, intEquals);
        CHECK(res != NULL && *res == vals[i], "каждый ключ корректно извлекается");
    }

    freeHashTable(t);
}

static void test_rehash(void)
{
    printf("Test: переполнение и rehash\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    int key, val;
    for (int i = 1; i <= 20; i++)
    {
        key = i;
        val = i * 10;
        setItemHashTable(t, &key, &val, HashInt, intEquals);
    }

    CHECK(t->size == 20, "все 20 элементов на месте");
    CHECK(t->capacity == 40, "capacity выросла до 40 (10 -> 20 -> 40)");

    for (int i = 1; i <= 20; i++)
    {
        key = i;
        int *res = getItemHashTable(t, &key, HashInt, intEquals);
        CHECK(res != NULL && *res == i * 10, "элемент доступен после rehash");
    }

    int missing = 1000;
    CHECK(getItemHashTable(t, &missing, HashInt, intEquals) == NULL,
          "несуществующий ключ после rehash возвращает NULL");

    /* capacity 40 и ключи 1..20 распределены без коллизий (primary = key) */
    CHECK(getCollisionCount(t, HashInt) == 0,
          "после rehash коллизий нет");

    freeHashTable(t);
}

static void test_delete_and_reinsert(void)
{
    printf("Test: удаление и повторная вставка\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    int key, val;

    /* снова ключи с общей коллизией: 1, 11, 21 */
    int keys[3] = {1, 11, 21};
    int vals[3] = {100, 110, 210};
    for (int i = 0; i < 3; i++)
    {
        setItemHashTable(t, &keys[i], &vals[i], HashInt, intEquals);
    }

    /* удаляем "средний" ключ 11 — слот помечается SLOT_DELETED */
    key = 11;
    int *pop = popItemHashTable(t, &key, HashInt, intEquals);
    CHECK(pop != NULL && *pop == 110, "pop возвращает копию значения");
    free(pop);
    CHECK(t->size == 2, "size == 2 после удаления");

    /* главная проверка DELETED-маркера: ключ 21, стоящий ЗА удалённым,
       должен оставаться находимым (напоминалка: 21 тоже хотел в слот 1) */
    key = 21;
    int *res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 210, "ключ после DELETED-слота всё ещё находится");

    /* повторная вставка: новый элемент должен попасть в освободившийся слот */
    key = 11;
    val = 1111;
    setItemHashTable(t, &key, &val, HashInt, intEquals);
    CHECK(t->size == 3, "при повторной вставке нового элемента size == 3");

    res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 1111, "повторно вставленный ключ читается с новым значением");

    key = 21;
    res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 210, "соседний ключ 21 не пострадал");

    freeHashTable(t);
}

static void test_typical_scenario(void)
{
    printf("Test: типичный сценарий (set/get/update/pop/reset)\n");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    int key = 42, val = 1;
    setItemHashTable(t, &key, &val, HashInt, intEquals);

    int *res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 1, "первое значение читается");

    /* обновление существующего ключа НЕ должно добавлять новый элемент */
    val = 2;
    setItemHashTable(t, &key, &val, HashInt, intEquals);
    CHECK(t->size == 1, "обновление ключа не меняет size");
    res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 2, "обновлённое значение читается");

    /* pop извлекает и убирает */
    int *pop = popItemHashTable(t, &key, HashInt, intEquals);
    CHECK(pop != NULL && *pop == 2, "pop возвращает значение");
    free(pop);
    CHECK(getItemHashTable(t, &key, HashInt, intEquals) == NULL,
          "после pop ключ не находится");
    CHECK(t->size == 0, "size == 0 после pop");

    /* вставка того же ключа заново после удаления */
    val = 3;
    setItemHashTable(t, &key, &val, HashInt, intEquals);
    res = getItemHashTable(t, &key, HashInt, intEquals);
    CHECK(res != NULL && *res == 3, "ключ после удаления снова работает");

    freeHashTable(t);
}

static void test_incorrect_input(void)
{
    printf("Test: некорректные входные данные\n");
    int key = 1, val = 2;

    CHECK(createHashTable(0, sizeof(int)) != NULL,
          "createHashTable с key_size = 0 не падает");
    CHECK(createHashTable(sizeof(int), 0) != NULL,
          "createHashTable с val_size = 0 не падает");

    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    CHECK(t != NULL, "createHashTable возвращает non-NULL");

    CHECK(getItemHashTable(NULL, &key, HashInt, intEquals) == NULL,
          "getItemHashTable(NULL, ...) возвращает NULL");
    CHECK(getItemHashTable(t, NULL, HashInt, intEquals) == NULL,
          "getItemHashTable(t, NULL, ...) возвращает NULL");
    CHECK(getItemHashTable(t, &key, NULL, intEquals) == NULL,
          "getItemHashTable с NULL hash возвращает NULL");
    CHECK(getItemHashTable(t, &key, HashInt, NULL) == NULL,
          "getItemHashTable с NULL cmp возвращает NULL");

    CHECK(popItemHashTable(NULL, &key, HashInt, intEquals) == NULL,
          "popItemHashTable(NULL, ...) возвращает NULL");
    CHECK(popItemHashTable(t, NULL, HashInt, intEquals) == NULL,
          "popItemHashTable(t, NULL, ...) возвращает NULL");
    CHECK(popItemHashTable(t, &key, NULL, intEquals) == NULL,
          "popItemHashTable с NULL hash возвращает NULL");
    CHECK(popItemHashTable(t, &key, HashInt, NULL) == NULL,
          "popItemHashTable с NULL cmp возвращает NULL");

    /* setItem возвращает void, поэтому проверяем только, что не падает */
    setItemHashTable(NULL, &key, &val, HashInt, intEquals);
    setItemHashTable(t, NULL, &val, HashInt, intEquals);
    setItemHashTable(t, &key, NULL, HashInt, intEquals);
    tests_passed++;
    printf("  [PASS] setItemHashTable с NULL-аргументами не падает\n");

    CHECK(getCollisionCount(NULL, HashInt) == 0,
          "getCollisionCount(NULL, ...) возвращает 0");
    CHECK(getCollisionCount(t, NULL) == 0,
          "getCollisionCount с NULL hash возвращает 0");

    setItemHashTable(t, &key, &val, HashInt, intEquals);
    freeHashTable(t);

    freeHashTable(NULL);
    tests_passed++;
    printf("  [PASS] freeHashTable(NULL) не падает\n");
}

static void test_string_keys(void)
{
    printf("Test: строковые ключи (HashString)\n");
    /* key_size = 32 байта: слот хранит копию C-строки (плюс хвост из нулей) */
    HashTable *t = createHashTable(32, sizeof(int));
    CHECK(t != NULL, "createHashTable для строк возвращает non-NULL");

    char k1[32] = "apple", k2[32] = "banana", k3[32] = "apple";
    int v1 = 1, v2 = 2, v3 = 3;

    setItemHashTable(t, k1, &v1, HashString, stringEquals);
    setItemHashTable(t, k2, &v2, HashString, stringEquals);
    setItemHashTable(t, k3, &v3, HashString, stringEquals); /* обновление "apple" */

    CHECK(t->size == 2, "обновление строкового ключа не добавляет новый элемент");

    int *r1 = getItemHashTable(t, k1, HashString, stringEquals);
    CHECK(r1 != NULL && *r1 == 3, "'apple' обновлён до 3");
    int *r2 = getItemHashTable(t, k2, HashString, stringEquals);
    CHECK(r2 != NULL && *r2 == 2, "'banana' == 2");

    char missing[32] = "cherry";
    CHECK(getItemHashTable(t, missing, HashString, stringEquals) == NULL,
          "несуществующая строка возвращает NULL");

    int *p = popItemHashTable(t, k1, HashString, stringEquals);
    CHECK(p != NULL && *p == 3, "pop строкового ключа возвращает значение");
    free(p);
    CHECK(getItemHashTable(t, k1, HashString, stringEquals) == NULL,
          "после pop строковый ключ не находится");

    freeHashTable(t);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== HashTable tests ===\n\n");

    test_empty_table();
    test_minimal_data();
    test_collisions();
    test_rehash();
    test_delete_and_reinsert();
    test_typical_scenario();
    test_incorrect_input();
    test_string_keys();

    printf("\n==========================\n");
    printf("Passed: %d, Failed: %d\n", tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}