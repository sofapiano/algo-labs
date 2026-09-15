#include "tasks.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../lab3/vector/generic.h"

/* ===========================================================================
   Вспомогательные функции (хелперы) — убирают повторяющийся код из тестов
   =========================================================================== */

/* Собирает Vector из массива C-строк. Вектор хранит не строки, а
   УКАЗАТЕЛИ на них (элемент вектора = char *). Если n == 0, strs может
   быть NULL — цикл просто не выполняется. */
static Vector *make_string_vector(const char **strs, size_t n)
{
    Vector *v = createVector(sizeof(char *));
    assert(v != NULL);
    for (size_t i = 0; i < n; i++)
    {
        assert(appendVectorItem(v, &strs[i]) == 0);
    }
    return v;
}

/* Достаёт C-строку (char *) из вектора строк на позиции i.
   В векторе лежат указатели, поэтому берём адрес слота и разыменовываем. */
static const char *get_str(Vector *v, size_t i)
{
    return *(const char **)getVectorItem(v, i);
}

/* Читает int из вектора int'ов на позиции i (для encodeStrings). */
static int get_code(Vector *v, size_t i)
{
    return *(int *)getVectorItem(v, i);
}

/* Короткая обёртка над setItemHashTable для int-ключей и int-значений. */
static void set_int(HashTable *t, int key, int val)
{
    setItemHashTable(t, &key, &val, HashInt, intEquals);
}

/* ===========================================================================
   Тесты для removeDuplicates
   =========================================================================== */

/* 1. Некорректный вход: NULL-вектор должен давать NULL, а не краш. */
static void test_remove_duplicates_null(void)
{
    printf("Test: removeDuplicates(NULL) ... ");
    assert(removeDuplicates(NULL) == NULL);
    printf("OK\n");
}

/* 2. Пустой вектор: результат должен быть пустым, но не NULL. */
static void test_remove_duplicates_empty(void)
{
    printf("Test: removeDuplicates пустого вектора ... ");
    Vector *in = make_string_vector(NULL, 0);
    Vector *out = removeDuplicates(in);
    assert(out != NULL);
    assert(out->size == 0);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* 3. Все строки уникальны: размер и порядок сохраняются. */
static void test_remove_duplicates_no_duplicates(void)
{
    printf("Test: removeDuplicates без дубликатов ... ");
    const char *strs[] = {"one", "two", "three"};
    Vector *in = make_string_vector(strs, 3);
    Vector *out = removeDuplicates(in);
    assert(out != NULL);
    assert(out->size == 3);
    assert(strcmp(get_str(out, 0), "one") == 0);
    assert(strcmp(get_str(out, 1), "two") == 0);
    assert(strcmp(get_str(out, 2), "three") == 0);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* 4. Дубликаты посередине и в конце: остаётся по одному вхождению,
      в порядке ПЕРВОГО появления. */
static void test_remove_duplicates_with_duplicates(void)
{
    printf("Test: removeDuplicates с дубликатами ... ");
    const char *strs[] = {"apple", "banana", "apple", "cherry", "banana"};
    Vector *in = make_string_vector(strs, 5);
    Vector *out = removeDuplicates(in);
    assert(out != NULL);
    assert(out->size == 3);
    assert(strcmp(get_str(out, 0), "apple") == 0);
    assert(strcmp(get_str(out, 1), "banana") == 0);
    assert(strcmp(get_str(out, 2), "cherry") == 0);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* 5. Все строки одинаковые: остаётся ровно одна. */
static void test_remove_duplicates_all_equal(void)
{
    printf("Test: removeDuplicates когда все строки одинаковые ... ");
    const char *strs[] = {"same", "same", "same"};
    Vector *in = make_string_vector(strs, 3);
    Vector *out = removeDuplicates(in);
    assert(out != NULL);
    assert(out->size == 1);
    assert(strcmp(get_str(out, 0), "same") == 0);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* ===========================================================================
   Тесты для encodeStrings
   =========================================================================== */

/* 1. Некорректный вход. */
static void test_encode_strings_null(void)
{
    printf("Test: encodeStrings(NULL) ... ");
    assert(encodeStrings(NULL) == NULL);
    printf("OK\n");
}

/* 2. Пустой вектор. */
static void test_encode_strings_empty(void)
{
    printf("Test: encodeStrings пустого вектора ... ");
    Vector *in = make_string_vector(NULL, 0);
    Vector *out = encodeStrings(in);
    assert(out != NULL);
    assert(out->size == 0);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* 3. Уникальные строки получают коды 0, 1, 2 ... в порядке вхождения. */
static void test_encode_strings_all_unique(void)
{
    printf("Test: encodeStrings с уникальными строками ... ");
    const char *strs[] = {"alpha", "beta", "gamma"};
    Vector *in = make_string_vector(strs, 3);
    Vector *out = encodeStrings(in);
    assert(out != NULL);
    assert(out->size == 3);
    assert(get_code(out, 0) == 0);
    assert(get_code(out, 1) == 1);
    assert(get_code(out, 2) == 2);
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* 4. Повторные строки должны получать ТОТ ЖЕ код, что и раньше.
      Новые строки — следующий по счёту код. */
static void test_encode_strings_with_duplicates(void)
{
    printf("Test: encodeStrings с повторяющимися строками ... ");
    const char *strs[] = {"x", "y", "x", "z", "y"};
    Vector *in = make_string_vector(strs, 5);
    Vector *out = encodeStrings(in);
    assert(out != NULL);
    assert(out->size == 5);
    assert(get_code(out, 0) == 0); /* первая встреча "x" -> 0 */
    assert(get_code(out, 1) == 1); /* первая встреча "y" -> 1 */
    assert(get_code(out, 2) == 0); /* снова "x" -> тот же 0 */
    assert(get_code(out, 3) == 2); /* новая "z" -> 2 */
    assert(get_code(out, 4) == 1); /* снова "y" -> тот же 1 */
    vectorFree(in);
    vectorFree(out);
    printf("OK\n");
}

/* ===========================================================================
   Тесты для swapKeysValues
   =========================================================================== */

/* 1. Некорректные аргументы: NULL-таблица, NULL-функция хеша или сравнения. */
static void test_swap_keys_values_null(void)
{
    printf("Test: swapKeysValues с NULL-аргументами ... ");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    assert(t != NULL);

    assert(swapKeysValues(NULL, HashInt, intEquals) == NULL);
    assert(swapKeysValues(t, NULL, intEquals) == NULL);
    assert(swapKeysValues(t, HashInt, NULL) == NULL);

    freeHashTable(t);
    printf("OK\n");
}

/* 2. Обычный обмен: {1->10, 2->20, 3->30} превращается в {10->1, 20->2, 30->3}. */
static void test_swap_keys_values_basic(void)
{
    printf("Test: swapKeysValues (1->10, 2->20, 3->30) ... ");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    assert(t != NULL);

    set_int(t, 1, 10);
    set_int(t, 2, 20);
    set_int(t, 3, 30);

    HashTable *swapped = swapKeysValues(t, HashInt, intEquals);
    assert(swapped != NULL);
    assert(swapped->size == 3);
    /* размеры ключа и значения должны поменяться местами */
    assert(swapped->key_size == sizeof(int));
    assert(swapped->val_size == sizeof(int));

    int key, *res;
    key = 10;
    res = getItemHashTable(swapped, &key, HashInt, intEquals);
    assert(res != NULL && *res == 1);

    key = 20;
    res = getItemHashTable(swapped, &key, HashInt, intEquals);
    assert(res != NULL && *res == 2);

    key = 30;
    res = getItemHashTable(swapped, &key, HashInt, intEquals);
    assert(res != NULL && *res == 3);

    /* ключа, которого не было среди значений, в новой таблице нет */
    key = 99;
    assert(getItemHashTable(swapped, &key, HashInt, intEquals) == NULL);

    /* исходная таблица должна остаться нетронутой */
    key = 1;
    res = getItemHashTable(t, &key, HashInt, intEquals);
    assert(res != NULL && *res == 10);

    freeHashTable(swapped);
    freeHashTable(t);
    printf("OK\n");
}

/* 3. Угловой случай: значения повторяются (1->10, 2->10).
      После обмена оба хотят стать ключом 10 — победит последний,
      в таблице останется одна пара. */
static void test_swap_keys_values_duplicate_values(void)
{
    printf("Test: swapKeysValues с одинаковыми значениями (1->10, 2->10) ... ");
    HashTable *t = createHashTable(sizeof(int), sizeof(int));
    assert(t != NULL);

    set_int(t, 1, 10);
    set_int(t, 2, 10);

    HashTable *swapped = swapKeysValues(t, HashInt, intEquals);
    assert(swapped != NULL);
    assert(swapped->size == 1);

    int key = 10;
    int *res = getItemHashTable(swapped, &key, HashInt, intEquals);
    assert(res != NULL);
    /* значение-ключ существует, а его значением стал один из исходных ключей */
    assert(*res == 1 || *res == 2);

    freeHashTable(swapped);
    freeHashTable(t);
    printf("OK\n");
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== tasks.c tests ===\n\n");

    printf("--- removeDuplicates ---\n");
    test_remove_duplicates_null();
    test_remove_duplicates_empty();
    test_remove_duplicates_no_duplicates();
    test_remove_duplicates_with_duplicates();
    test_remove_duplicates_all_equal();

    printf("\n--- encodeStrings ---\n");
    test_encode_strings_null();
    test_encode_strings_empty();
    test_encode_strings_all_unique();
    test_encode_strings_with_duplicates();

    printf("\n--- swapKeysValues ---\n");
    test_swap_keys_values_null();
    test_swap_keys_values_basic();
    test_swap_keys_values_duplicate_values();

    printf("\nВсе тесты пройдены!\n");
    return 0;
}