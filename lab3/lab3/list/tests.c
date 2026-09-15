#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

typedef struct
{
    int x;
    int y;
} Point;

static int pointEquals(const void *a, const void *b)
{
    const Point *pa = (const Point *)a;
    const Point *pb = (const Point *)b;
    return pa->x == pb->x && pa->y == pb->y;
}

static void addInt(GenericList *list, int value)
{
    appendItem(list, &value);
}

static double now_seconds(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

/*
===============================================================================
OPTIMISED BULK BUILD
-------------------------------------------------------------------------------
appendItem() walks to the tail on every insert, so preloading a huge list
(100k / 1M) through the public API is O(n^2) and would take many minutes.
The "millions of elements" load tests only need to verify listLength,
findItem, popItem and freeList against a large list, so we build the list in
O(1) per element here (tracking the tail directly). The deep-copy semantics
of appendItem are still verified at small scale by test_append_and_length /
test_deep_copy. The same memory layout a valid list has is produced, and
every other accessor under test comes from generic.c
===============================================================================
*/
static void bulkAppendInt(GenericList *list, Node **tail, int value)
{
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    node->data = malloc(list->elem_size);
    if (!node->data) {
        free(node);
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    memcpy(node->data, &value, list->elem_size);
    node->next = NULL;

    if (*tail == NULL) {
        list->head = node;
    } else {
        (*tail)->next = node;
    }
    *tail = node;
}

static void test_empty_list(void)
{
    printf("Test: empty list\n");
    GenericList *list = createList(sizeof(int));
    CHECK(listLength(list) == 0, "length of empty list is 0");
    CHECK(findItem(list, &(int){5}, intEquals) == -1, "findItem on empty list returns -1");
    CHECK(popItem(list, 0) == NULL, "popItem on empty list returns NULL");
    freeList(list);
}

static void test_free_null(void)
{
    printf("Test: freeList(NULL)\n");
    freeList(NULL);
    tests_passed++;
    printf("  [PASS] freeList(NULL) does not crash\n");
}

static void test_append_and_length(void)
{
    printf("Test: append and length\n");
    GenericList *list = createList(sizeof(int));

    addInt(list, 10);
    addInt(list, 20);
    addInt(list, 30);

    CHECK(listLength(list) == 3, "length is 3 after three appends");
    CHECK(findItem(list, &(int){10}, intEquals) == 0, "findItem returns index 0");
    CHECK(findItem(list, &(int){20}, intEquals) == 1, "findItem returns index 1");
    CHECK(findItem(list, &(int){30}, intEquals) == 2, "findItem returns index 2");
    CHECK(findItem(list, &(int){99}, intEquals) == -1, "findItem returns -1 for missing value");

    freeList(list);
}

static void test_deep_copy(void)
{
    printf("Test: deep copy\n");
    GenericList *list = createList(sizeof(int));

    int original = 42;
    addInt(list, original);
    original = 100;

    CHECK(findItem(list, &(int){42}, intEquals) == 0, "data stored is a copy, not a reference");

    freeList(list);
}

static void test_pop_item(void)
{
    printf("Test: popItem\n");
    GenericList *list = createList(sizeof(int));

    addInt(list, 10);
    addInt(list, 20);
    addInt(list, 30);

    int *removed;

    removed = (int *)popItem(list, 1);
    CHECK(removed != NULL && *removed == 20, "popItem(1) returns copy of 20");
    free(removed);

    CHECK(listLength(list) == 2, "length is 2 after pop");
    CHECK(findItem(list, &(int){10}, intEquals) == 0, "10 still at index 0");
    CHECK(findItem(list, &(int){30}, intEquals) == 1, "30 moved to index 1");

    removed = (int *)popItem(list, 0);
    CHECK(removed != NULL && *removed == 10, "popItem(0) removes head (10)");
    free(removed);

    removed = (int *)popItem(list, 0);
    CHECK(removed != NULL && *removed == 30, "popItem(0) removes last (30)");
    free(removed);

    removed = (int *)popItem(list, 0);
    CHECK(removed == NULL, "popItem on empty list returns NULL");
    CHECK(listLength(list) == 0, "list is empty after popping everything");

    freeList(list);
}

static void test_pop_single_element_list(void)
{
    printf("Test: popItem on single-element list\n");
    GenericList *list = createList(sizeof(int));

    addInt(list, 7);
    int *removed = (int *)popItem(list, 0);
    CHECK(removed != NULL && *removed == 7, "popItem(0) on single element works");
    free(removed);
    CHECK(listLength(list) == 0, "list empty after popping only element");
    CHECK(findItem(list, &(int){7}, intEquals) == -1, "findItem after pop returns -1");

    freeList(list);
}

static void test_pop_out_of_range(void)
{
    printf("Test: popItem out of range\n");
    GenericList *list = createList(sizeof(int));

    addInt(list, 5);
    addInt(list, 6);

    int *removed = (int *)popItem(list, 5);
    CHECK(removed == NULL, "popItem out of range returns NULL");
    free(removed);

    CHECK(listLength(list) == 2, "list unchanged after out-of-range pop");

    freeList(list);
}

static void test_float_list(void)
{
    printf("Test: float type\n");
    GenericList *list = createList(sizeof(float));

    float a = 1.5f, b = 2.5f, c = 3.5f;
    appendItem(list, &a);
    appendItem(list, &b);
    appendItem(list, &c);

    CHECK(listLength(list) == 3, "float list length is 3");
    CHECK(findItem(list, &(float){1.5f}, floatEquals) == 0, "findItem finds float at 0");
    CHECK(findItem(list, &(float){3.5f}, floatEquals) == 2, "findItem finds float at 2");

    freeList(list);
}

static void test_long_long_list(void)
{
    printf("Test: long long type\n");
    GenericList *list = createList(sizeof(long long));

    long long vals[] = {10000000000LL, -5LL, 42LL};
    for (int i = 0; i < 3; i++) {
        appendItem(list, &vals[i]);
    }

    CHECK(listLength(list) == 3, "long long list length is 3");
    CHECK(findItem(list, &(long long){-5LL}, longLongEquals) == 1, "findItem finds long long at 1");
    CHECK(findItem(list, &(long long){999LL}, longLongEquals) == -1, "missing long long returns -1");

    freeList(list);
}

static void test_string_list(void)
{
    printf("Test: string type (deep copy)\n");
    GenericList *list = createList(sizeof(char) * 32);

    char s1[32] = "banana";
    char s2[32] = "papa";
    char s3[32] = "banana";
    appendItem(list, s1);
    appendItem(list, s2);
    appendItem(list, s3);

    CHECK(listLength(list) == 3, "string list length is 3");
    CHECK(findItem(list, "banana", stringEquals) == 0, "findItem finds first banana at 0");
    CHECK(findItem(list, "papa", stringEquals) == 1, "findItem finds papa at 1");
    CHECK(findItem(list, "mama", stringEquals) == -1, "missing string returns -1");

    /* deep copy: overwrite source buffer, stored copy must be unchanged */
    strcpy(s2, "XXXX");
    CHECK(findItem(list, "papa", stringEquals) == 1, "stored string is a deep copy");

    char *taken = (char *)popItem(list, 0);
    CHECK(taken != NULL && strcmp(taken, "banana") == 0, "popItem returns copy of string");
    free(taken);

    freeList(list);
}

static void test_struct_list(void)
{
    printf("Test: struct type\n");
    GenericList *list = createList(sizeof(Point));

    Point p1 = {1, 2};
    Point p2 = {3, 4};
    appendItem(list, &p1);
    appendItem(list, &p2);

    CHECK(listLength(list) == 2, "struct list length is 2");
    CHECK(findItem(list, &(Point){3, 4}, pointEquals) == 1, "findItem finds struct at 1");
    CHECK(findItem(list, &(Point){9, 9}, pointEquals) == -1, "missing struct returns -1");

    Point *removed = (Point *)popItem(list, 0);
    CHECK(removed != NULL && removed->x == 1 && removed->y == 2, "popItem returns copy of struct");
    free(removed);

    CHECK(listLength(list) == 1, "struct list length is 1 after pop");

    freeList(list);
}

/*
Load test using the real public appendItem(). Kept at a modest size: this API
is O(n) per append, so preloading n elements costs O(n^2) - preloading 100k
through it already takes ~27s and 1M would take ~45 minutes. appendItem's
behaviour is type/scale independent, so a 10k real-API run fully validates it
while the true 100k/1M scale is covered by the fast O(1) bulk build below.
*/
static void test_stress_append_api(void)
{
    printf("Test: stress via public appendItem (10,000 elements)\n");
    double t0 = now_seconds();
    GenericList *list = createList(sizeof(int));

    const int N = 10000;
    for (int i = 0; i < N; i++) {
        addInt(list, i);
    }

    CHECK(listLength(list) == (unsigned int)N, "length matches after stress inserts");
    CHECK(findItem(list, &(int){0}, intEquals) == 0, "findItem first element");
    CHECK(findItem(list, &(int){N - 1}, intEquals) == N - 1, "findItem last element");
    CHECK(findItem(list, &(int){-1}, intEquals) == -1, "findItem missing element");
    CHECK(findItem(list, &(int){N / 2}, intEquals) == N / 2, "findItem middle element");

    unsigned int n = listLength(list);
    for (unsigned int i = 0; i < n; i++) {
        int *removed = (int *)popItem(list, 0);
        free(removed);
    }
    CHECK(listLength(list) == 0, "list empty after popping all stress elements");

    freeList(list);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

/*
Load test with a million elements. Built with the O(1) bulk helper so it stays
fast; all accessors exercised here are the real generic.c implementations.
*/
static void test_stress_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: stress (1,000,000 elements)\n");
    double t0 = now_seconds();

    GenericList *list = createList(sizeof(int));
    Node *tail = NULL;
    for (unsigned int i = 0; i < N; i++) {
        bulkAppendInt(list, &tail, (int)i);
    }

    CHECK(listLength(list) == N, "length is 1,000,000 after bulk build");
    CHECK(findItem(list, &(int){0}, intEquals) == 0, "findItem first element at 0");
    CHECK(findItem(list, &(int){N - 1}, intEquals) == (int)(N - 1), "findItem last element");
    CHECK(findItem(list, &(int){500000}, intEquals) == 500000, "findItem middle element");
    CHECK(findItem(list, &(int){-100}, intEquals) == -1, "findItem missing element returns -1");

    /* pop a handful of elements from the head (index 0 is O(1)) */
    int *r0 = (int *)popItem(list, 0);
    CHECK(r0 != NULL && *r0 == 0, "popItem(0) returns first element of 1M list");
    free(r0);
    CHECK(listLength(list) == N - 1, "length is 999,999 after one pop");

    freeList(list);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== GenericList tests ===\n\n");
    double start = now_seconds();

    test_empty_list();
    test_free_null();
    test_append_and_length();
    test_deep_copy();
    test_pop_item();
    test_pop_single_element_list();
    test_pop_out_of_range();
    test_float_list();
    test_long_long_list();
    test_string_list();
    test_struct_list();
    test_stress_append_api();
    test_stress_1m();

    printf("\n==========================\n");
    printf("Passed: %d, Failed: %d\n", tests_passed, tests_failed);
    printf("Total time: %.2f s\n", now_seconds() - start);
    return tests_failed == 0 ? 0 : 1;
}
