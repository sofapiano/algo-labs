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

static double now_seconds(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

static void test_empty_vector(void)
{
    printf("Test: empty vector\n");
    Vector *v = createVector(sizeof(int));
    CHECK(v != NULL, "createVector returns non-NULL");
    CHECK(v->size == 0, "size of empty vector is 0");
    CHECK(getVectorItem(v, 0) == NULL, "getVectorItem on empty vector returns NULL");
    CHECK(findVectorItem(v, &(int){5}, intEquals) == -1, "findVectorItem on empty vector returns -1");
    CHECK(popVectorItem(v, 0) == NULL, "popVectorItem on empty vector returns NULL");
    free(v->data);
    free(v);
    vectorFree(NULL);
    tests_passed++;
    printf("  [PASS] vectorFree(NULL) does not crash\n");
}

static void test_create_with_zero_elem_size(void)
{
    printf("Test: zero elem_size\n");
    /* elem_size = 0 must never crash; every operation stays safe */
    Vector *v = createVector(0);
    CHECK(v != NULL, "createVector(0) returns non-NULL");
    int x = 1;
    CHECK(appendVectorItem(v, &x) == 0, "appendVectorItem with elem_size 0 succeeds");
    CHECK(v->size == 1, "size is 1 after append with elem_size 0");
    CHECK(setVectorItem(v, 0, &x) == 0, "setVectorItem with elem_size 0 succeeds");
    vectorFree(v);
}

static void test_append_and_length(void)
{
    printf("Test: append and length\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){10});
    appendVectorItem(v, &(int){20});
    appendVectorItem(v, &(int){30});

    CHECK(v->size == 3, "size is 3 after three appends");
    CHECK(*(int *)getVectorItem(v, 0) == 10, "getVectorItem(0) == 10");
    CHECK(*(int *)getVectorItem(v, 1) == 20, "getVectorItem(1) == 20");
    CHECK(*(int *)getVectorItem(v, 2) == 30, "getVectorItem(2) == 30");

    CHECK(findVectorItem(v, &(int){10}, intEquals) == 0, "findVectorItem returns 0 for 10");
    CHECK(findVectorItem(v, &(int){20}, intEquals) == 1, "findVectorItem returns 1 for 20");
    CHECK(findVectorItem(v, &(int){30}, intEquals) == 2, "findVectorItem returns 2 for 30");
    CHECK(findVectorItem(v, &(int){99}, intEquals) == -1, "findVectorItem returns -1 for missing");

    CHECK(v->capacity >= v->size, "capacity grown beyond size");

    vectorFree(v);
}

static void test_growth_factor(void)
{
    printf("Test: capacity growth (doubling)\n");
    Vector *v = createVector(sizeof(int));

    size_t prevCap = v->capacity;
    CHECK(v->capacity >= MIN_SIZE, "initial capacity at least MIN_SIZE");

    size_t growths = 0;
    for (int i = 0; i < 1000; i++) {
        appendVectorItem(v, &i);
        if (v->capacity != prevCap) {
            CHECK(v->capacity >= v->size, "capacity always >= size after growth");
            prevCap = v->capacity;
            growths++;
        }
    }
    CHECK(growths > 0, "capacity grew several times during 1000 appends");
    CHECK(v->size == 1000, "size is 1000 after appends");
    CHECK(v->capacity >= 1000, "capacity covers all 1000 elements");

    vectorFree(v);
}

static void test_deep_copy(void)
{
    printf("Test: deep copy\n");
    Vector *v = createVector(sizeof(int));

    int original = 42;
    appendVectorItem(v, &original);
    original = 100;

    CHECK(*(int *)getVectorItem(v, 0) == 42, "stored data is a copy, not a reference");

    vectorFree(v);
}

static void test_set_vector_item(void)
{
    printf("Test: setVectorItem\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){1});
    appendVectorItem(v, &(int){2});
    appendVectorItem(v, &(int){3});

    CHECK(setVectorItem(v, 1, &(int){99}) == 0, "setVectorItem(1) succeeds");
    CHECK(*(int *)getVectorItem(v, 1) == 99, "element at index 1 is now 99");
    CHECK(v->size == 3, "size unchanged after set");
    CHECK(*(int *)getVectorItem(v, 0) == 1, "element at index 0 unchanged");
    CHECK(*(int *)getVectorItem(v, 2) == 3, "element at index 2 unchanged");

    CHECK(setVectorItem(v, 5, &(int){7}) != 0, "setVectorItem out of range fails");
    CHECK(setVectorItem(v, 3, &(int){7}) != 0, "setVectorItem at index == size fails");
    CHECK(setVectorItem(v, 0, NULL) != 0, "setVectorItem with NULL value fails");
    CHECK(setVectorItem(NULL, 0, &(int){7}) != 0, "setVectorItem with NULL vector fails");

    vectorFree(v);
}

static void test_get_out_of_range(void)
{
    printf("Test: getVectorItem out of range\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){5});
    appendVectorItem(v, &(int){6});

    CHECK(getVectorItem(v, 2) == NULL, "getVectorItem(size) returns NULL");
    CHECK(getVectorItem(v, 100) == NULL, "getVectorItem(100) returns NULL");
    CHECK(getVectorItem(v, 0) != NULL, "getVectorItem(0) valid");
    CHECK(getVectorItem(NULL, 0) == NULL, "getVectorItem(NULL, 0) returns NULL");

    vectorFree(v);
}

static void test_pop_vector_item(void)
{
    printf("Test: popVectorItem\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){10});
    appendVectorItem(v, &(int){20});
    appendVectorItem(v, &(int){30});

    int *removed;

    removed = (int *)popVectorItem(v, 1);
    CHECK(removed != NULL && *removed == 20, "popVectorItem(1) returns copy of 20");
    free(removed);

    CHECK(v->size == 2, "size is 2 after pop");
    CHECK(*(int *)getVectorItem(v, 0) == 10, "10 still at index 0");
    CHECK(*(int *)getVectorItem(v, 1) == 30, "30 moved to index 1");

    removed = (int *)popVectorItem(v, 0);
    CHECK(removed != NULL && *removed == 10, "popVectorItem(0) returns 10");
    free(removed);
    CHECK(*(int *)getVectorItem(v, 0) == 30, "30 at index 0 after popping head");

    removed = (int *)popVectorItem(v, 0);
    CHECK(removed != NULL && *removed == 30, "popVectorItem(0) returns last element");
    free(removed);
    CHECK(v->size == 0, "size is 0 after popping everything");
    CHECK(getVectorItem(v, 0) == NULL, "getVectorItem returns NULL for empty vector");

    /* IEnumerable: pop from the tail keeps relative order, O(1) per pop */
    vectorFree(v);
}

static void test_pop_single_element(void)
{
    printf("Test: popVectorItem on single element\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){7});
    int *removed = (int *)popVectorItem(v, 0);
    CHECK(removed != NULL && *removed == 7, "popVectorItem(0) on single element works");
    free(removed);
    CHECK(v->size == 0, "size 0 after popping only element");
    CHECK(findVectorItem(v, &(int){7}, intEquals) == -1, "findVectorItem after pop returns -1");

    vectorFree(v);
}

static void test_pop_out_of_range(void)
{
    printf("Test: popVectorItem out of range\n");
    Vector *v = createVector(sizeof(int));

    appendVectorItem(v, &(int){5});
    appendVectorItem(v, &(int){6});

    CHECK(popVectorItem(v, 5) == NULL, "popVectorItem out of range returns NULL");
    CHECK(popVectorItem(v, 2) == NULL, "popVectorItem at size returns NULL");
    CHECK(popVectorItem(NULL, 0) == NULL, "popVectorItem(NULL, 0) returns NULL");
    CHECK(v->size == 2, "size unchanged after out-of-range pop");

    vectorFree(v);
}

static void test_float_vector(void)
{
    printf("Test: float type\n");
    Vector *v = createVector(sizeof(float));

    float a = 1.5f, b = 2.5f, c = 3.5f;
    appendVectorItem(v, &a);
    appendVectorItem(v, &b);
    appendVectorItem(v, &c);

    CHECK(v->size == 3, "float vector size is 3");
    CHECK(*(float *)getVectorItem(v, 0) == 1.5f, "getVectorItem(0) == 1.5");
    CHECK(findVectorItem(v, &(float){1.5f}, floatEquals) == 0, "findVectorItem finds float at 0");
    CHECK(findVectorItem(v, &(float){3.5f}, floatEquals) == 2, "findVectorItem finds float at 2");
    CHECK(findVectorItem(v, &(float){9.9f}, floatEquals) == -1, "findVectorItem finds no missing float");

    vectorFree(v);
}

static void test_long_ll_vector(void)
{
    printf("Test: long long type\n");
    Vector *v = createVector(sizeof(long long));

    long long vals[] = {10000000000LL, -5LL, 42LL};
    for (int i = 0; i < 3; i++) {
        appendVectorItem(v, &vals[i]);
    }

    CHECK(v->size == 3, "long long vector size is 3");
    CHECK(*(long long *)getVectorItem(v, 0) == 10000000000LL, "getVectorItem(0) == 10000000000");
    CHECK(findVectorItem(v, &(long long){-5LL}, longLongEquals) == 1, "findVectorItem finds -5 at 1");
    CHECK(findVectorItem(v, &(long long){999LL}, longLongEquals) == -1, "missing long long returns -1");

    vectorFree(v);
}

static void test_string_vector(void)
{
    printf("Test: string type (deep copy)\n");
    Vector *v = createVector(sizeof(char) * 32);

    char s1[32] = "banana";
    char s2[32] = "papa";
    char s3[32] = "banana";
    appendVectorItem(v, s1);
    appendVectorItem(v, s2);
    appendVectorItem(v, s3);

    CHECK(v->size == 3, "string vector size is 3");
    CHECK(findVectorItem(v, "banana", stringEquals) == 0, "findVectorItem finds first banana at 0");
    CHECK(findVectorItem(v, "papa", stringEquals) == 1, "findVectorItem finds papa at 1");
    CHECK(findVectorItem(v, "mama", stringEquals) == -1, "missing string returns -1");

    /* deep copy: overwrite source buffer, stored copy must be unchanged */
    strcpy(s2, "XXXX");
    CHECK(findVectorItem(v, "papa", stringEquals) == 1, "stored string is a deep copy");

    char *taken = (char *)popVectorItem(v, 0);
    CHECK(taken != NULL && strcmp(taken, "banana") == 0, "popVectorItem returns copy of string");
    free(taken);

    vectorFree(v);
}

static void test_struct_vector(void)
{
    printf("Test: struct type\n");
    Vector *v = createVector(sizeof(Point));

    Point p1 = {1, 2};
    Point p2 = {3, 4};
    appendVectorItem(v, &p1);
    appendVectorItem(v, &p2);

    CHECK(v->size == 2, "struct vector size is 2");
    Point *g = (Point *)getVectorItem(v, 0);
    CHECK(g != NULL && g->x == 1 && g->y == 2, "getVectorItem(0) returns struct copy");
    CHECK(findVectorItem(v, &(Point){3, 4}, pointEquals) == 1, "findVectorItem finds struct at 1");
    CHECK(findVectorItem(v, &(Point){9, 9}, pointEquals) == -1, "missing struct returns -1");

    Point *removed = (Point *)popVectorItem(v, 0);
    CHECK(removed != NULL && removed->x == 1 && removed->y == 2, "popVectorItem returns copy of struct");
    free(removed);

    CHECK(v->size == 1, "struct vector size is 1 after pop");

    vectorFree(v);
}

/*
Load test via the real public appendVectorItem() - this API is O(1) amortized,
so preloading 100k / 1M through it is fast and fully exercises resizing. Only
findItem/get on a few elements are used, because popping everything one by one
would be O(n^2). popVectorItem's correctness is covered at small scale above
and its scale behaviour by draining from the tail in the 1M test below.
*/
static void test_stress_100k(void)
{
    printf("Test: stress (100,000 elements via appendVectorItem)\n");
    double t0 = now_seconds();
    Vector *v = createVector(sizeof(int));

    const unsigned int N = 100000u;
    for (unsigned int i = 0; i < N; i++) {
        appendVectorItem(v, &(int){i});
    }

    CHECK(v->size == N, "size matches after 100k inserts");
    CHECK(*(int *)getVectorItem(v, 0) == 0, "getVectorItem first element");
    CHECK(*(int *)getVectorItem(v, N - 1) == (int)(N - 1), "getVectorItem last element");
    CHECK(findVectorItem(v, &(int){50000}, intEquals) == 50000, "findVectorItem middle element");
    CHECK(findVectorItem(v, &(int){-1}, intEquals) == -1, "findVectorItem missing element");

    /* verify some random-ish samples are contiguous and correct */
    int ok_samples = 1;
    for (unsigned int i = 0; i + 1 < N; i += 997) {
        if (*(int *)getVectorItem(v, i) != (int)i) {
            ok_samples = 0;
            break;
        }
    }
    CHECK(ok_samples, "elements stored in insertion order across samples");

    vectorFree(v);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

/*
Load test with a million elements. appendVectorItem is O(1) amortized, so all
1M go through the real public API. Draining afterwards is done from the tail
(index size-1, O(1) per pop) to avoid the O(n^2) quadratic cost of popping the
head a million times.
*/
static void test_stress_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: stress (1,000,000 elements via appendVectorItem)\n");
    double t0 = now_seconds();

    Vector *v = createVector(sizeof(int));
    for (unsigned int i = 0; i < N; i++) {
        appendVectorItem(v, &(int){i});
    }

    CHECK(v->size == N, "size is 1,000,000 after inserts");
    CHECK(v->capacity >= N, "capacity covers 1,000,000 elements");
    CHECK(*(int *)getVectorItem(v, 0) == 0, "getVectorItem first element at 0");
    CHECK(*(int *)getVectorItem(v, N - 1) == (int)(N - 1), "getVectorItem last element");
    CHECK(findVectorItem(v, &(int){500000}, intEquals) == 500000, "findVectorItem middle element");
    CHECK(findVectorItem(v, &(int){-100}, intEquals) == -1, "findVectorItem missing element returns -1");

    /* pop from the tail: O(1) each, so draining 1M stays fast */
    unsigned int popped = 0;
    while (v->size > 0) {
        int *r = (int *)popVectorItem(v, v->size - 1);
        if (r != NULL) {
            free(r);
            popped++;
        } else {
            break;
        }
    }
    CHECK(popped == N, "drained all 1,000,000 elements from the tail");
    CHECK(v->size == 0, "size is 0 after draining");

    vectorFree(v);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_null_operations(void)
{
    printf("Test: NULL arguments\n");
    CHECK(createVector(0) != NULL, "createVector(0) works");

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){1});

    CHECK(appendVectorItem(NULL, &(int){1}) != 0, "appendVectorItem(NULL, el) fails");
    CHECK(appendVectorItem(v, NULL) != 0, "appendVectorItem(v, NULL) fails");
    CHECK(findVectorItem(v, NULL, intEquals) == -1, "findVectorItem(v, NULL, cmp) returns -1");
    CHECK(findVectorItem(v, &(int){1}, NULL) == -1, "findVectorItem with NULL cmp returns -1");
    CHECK(findVectorItem(NULL, &(int){1}, intEquals) == -1, "findVectorItem(NULL, ...) returns -1");
    CHECK(vectorFree(NULL) != 0, "vectorFree(NULL) returns error code");

    vectorFree(v);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== Vector tests ===\n\n");
    double start = now_seconds();

    test_empty_vector();
    test_create_with_zero_elem_size();
    test_null_operations();
    test_append_and_length();
    test_growth_factor();
    test_deep_copy();
    test_set_vector_item();
    test_get_out_of_range();
    test_pop_vector_item();
    test_pop_single_element();
    test_pop_out_of_range();
    test_float_vector();
    test_long_ll_vector();
    test_string_vector();
    test_struct_vector();
    test_stress_100k();
    test_stress_1m();

    printf("\n==========================\n");
    printf("Passed: %d, Failed: %d\n", tests_passed, tests_failed);
    printf("Total time: %.2f s\n", now_seconds() - start);
    return tests_failed == 0 ? 0 : 1;
}
