#include "base_tasks.h"
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

/*
===============================================================================
OPTIMISED BULK BUILD (list)
------------------------------------------------------------------------------
appendItem() walks to the tail on every insert, so preloading a huge list
(100k / 1M) through the public API is O(n^2) and would take many minutes.
The "million elements" load tests only need a long, correctly built list to
feed findMaxStudent / removeDuplicatesList, so we build it in O(1) per element
here while keeping the exact same memory layout appendItem() produces.
appendItem's behaviour itself is verified at small scale in the other tests.
===============================================================================
*/
static void bulkAppendStudent(GenericList *list, Node **tail, const Student *s)
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
    memcpy(node->data, s, list->elem_size);
    node->next = NULL;

    if (*tail == NULL) {
        list->head = node;
    } else {
        (*tail)->next = node;
    }
    *tail = node;
}

static void addStudent(GenericList *list, const char *name, float avg)
{
    Student s;
    strncpy(s.name, name, sizeof(s.name) - 1);
    s.name[sizeof(s.name) - 1] = '\0';
    s.avg = avg;
    appendItem(list, &s);
}

static void test_find_max_student_basic(void)
{
    printf("Test: findMaxStudent basic\n");

    CHECK(findMaxStudent(NULL) == NULL, "findMaxStudent(NULL) returns NULL");

    GenericList *empty = createList(sizeof(Student));
    CHECK(findMaxStudent(empty) == NULL, "findMaxStudent(empty list) returns NULL");
    freeList(empty);

    GenericList *list = createList(sizeof(Student));
    addStudent(list, "Ivanov", 4.2f);
    addStudent(list, "Petrov", 3.7f);
    addStudent(list, "Sidorov", 4.85f);
    addStudent(list, "Kuznetsov", 3.1f);

    Student *max = findMaxStudent(list);
    CHECK(max != NULL, "findMaxStudent returns non-NULL");
    CHECK(max != NULL && strcmp(max->name, "Sidorov") == 0, "returns student with max avg");
    CHECK(max != NULL && max->avg == 4.85f, "returns correct avg value");

    freeList(list);
}

static void test_find_max_student_edges(void)
{
    printf("Test: findMaxStudent edge cases\n");

    GenericList *list = createList(sizeof(Student));

    addStudent(list, "Lonely", 2.5f);
    Student *only = findMaxStudent(list);
    CHECK(only != NULL && strcmp(only->name, "Lonely") == 0, "single student is the max");
    freeList(list);

    list = createList(sizeof(Student));
    addStudent(list, "First", 5.0f);
    addStudent(list, "Second", 3.0f);
    CHECK(strcmp(findMaxStudent(list)->name, "First") == 0, "max at head is found");
    freeList(list);

    list = createList(sizeof(Student));
    addStudent(list, "First", 3.0f);
    addStudent(list, "Second", 5.0f);
    CHECK(strcmp(findMaxStudent(list)->name, "Second") == 0, "max at tail is found");
    freeList(list);

    list = createList(sizeof(Student));
    addStudent(list, "NegA", -1.5f);
    addStudent(list, "NegB", -2.5f);
    Student *neg = findMaxStudent(list);
    CHECK(neg != NULL && strcmp(neg->name, "NegA") == 0, "handles negative avgs (max -1.5)");
    freeList(list);

    list = createList(sizeof(Student));
    addStudent(list, "TieA", 4.0f);
    addStudent(list, "TieB", 4.0f);
    Student *tie = findMaxStudent(list);
    CHECK(tie != NULL && strcmp(tie->name, "TieA") == 0, "on tie returns first occurrence");
    freeList(list);
}

static void test_find_max_student_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: findMaxStudent (1,000,000 students)\n");
    double t0 = now_seconds();

    GenericList *list = createList(sizeof(Student));
    Node *tail = NULL;
    for (unsigned int i = 0; i < N; i++) {
        Student s;
        snprintf(s.name, sizeof(s.name), "Student_%u", i);
        s.avg = (float)i;
        bulkAppendStudent(list, &tail, &s);
    }

    Student *max = findMaxStudent(list);
    CHECK(max != NULL, "findMaxStudent returns non-NULL on 1M list");
    CHECK(max != NULL && max->avg == (float)(N - 1), "max avg is the last element (N-1)");
    CHECK(max != NULL && strcmp(max->name, "Student_999999") == 0, "returns correct name on 1M list");

    freeList(list);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_find_max_vector_basic(void)
{
    printf("Test: findMaxVector basic (int)\n");

    CHECK(findMaxVector(NULL, intGreater) == NULL, "findMaxVector(NULL) returns NULL");

    Vector *empty = createVector(sizeof(int));
    CHECK(findMaxVector(empty, intGreater) == NULL, "findMaxVector(empty) returns NULL");
    vectorFree(empty);

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){5});
    appendVectorItem(v, &(int){2});
    appendVectorItem(v, &(int){9});
    appendVectorItem(v, &(int){1});
    appendVectorItem(v, &(int){7});

    int *max = (int *)findMaxVector(v, intGreater);
    CHECK(max != NULL && *max == 9, "finds max int value 9");
    free(max);

    vectorFree(v);
}

static void test_find_max_vector_types(void)
{
    printf("Test: findMaxVector across types\n");

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){-10});
    appendVectorItem(v, &(int){-3});
    appendVectorItem(v, &(int){-7});
    int *ni = (int *)findMaxVector(v, intGreater);
    CHECK(ni != NULL && *ni == -3, "max of negative ints is -3");
    free(ni);
    vectorFree(v);

    v = createVector(sizeof(long));
    long lon[] = {100L, -5L, 42L};
    for (int i = 0; i < 3; i++) appendVectorItem(v, &lon[i]);
    long *lm = (long *)findMaxVector(v, longGreater);
    CHECK(lm != NULL && *lm == 100L, "max long is 100");
    free(lm);
    vectorFree(v);

    v = createVector(sizeof(float));
    float fl[] = {1.5f, 9.9f, 3.3f};
    for (int i = 0; i < 3; i++) appendVectorItem(v, &fl[i]);
    float *fm = (float *)findMaxVector(v, floatGreater);
    CHECK(fm != NULL && *fm == 9.9f, "max float is 9.9");
    free(fm);
    vectorFree(v);
}

static void test_find_max_vector_deep_copy(void)
{
    printf("Test: findMaxVector returns a deep copy\n");

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){3});
    appendVectorItem(v, &(int){8});

    int *res = (int *)findMaxVector(v, intGreater);
    CHECK(res != NULL && *res == 8, "result value correct");
    CHECK(res != NULL && (void *)res != getVectorItem(v, 1), "result is a separate allocation");

    setVectorItem(v, 1, &(int){100});
    CHECK(res != NULL && *res == 8, "copy unaffected by later source mutation");
    free(res);

    vectorFree(v);
}

static void test_find_max_vector_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: findMaxVector (1,000,000 ints)\n");
    double t0 = now_seconds();

    Vector *v = createVector(sizeof(int));
    for (unsigned int i = 0; i < N; i++) {
        appendVectorItem(v, &(int){i});
    }

    int *max = (int *)findMaxVector(v, intGreater);
    CHECK(max != NULL && *max == (int)(N - 1), "max of 1M ints is N-1");
    free(max);

    vectorFree(v);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_remove_dup_list_basic(void)
{
    printf("Test: removeDuplicatesList basic\n");

    CHECK(removeDuplicatesList(NULL, intEquals) == -1, "NULL list returns -1");

    GenericList *empty = createList(sizeof(int));
    CHECK(removeDuplicatesList(empty, intEquals) == -1, "empty list returns -1");
    freeList(empty);

    GenericList *list = createList(sizeof(int));
    int vals[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) appendItem(list, &vals[i]);
    CHECK(removeDuplicatesList(list, intEquals) == 0, "all-unique list returns 0");
    CHECK(listLength(list) == 3, "all-unique list unchanged (length 3)");
    freeList(list);

    list = createList(sizeof(int));
    int dup[] = {1, 1, 1};
    for (int i = 0; i < 3; i++) appendItem(list, &dup[i]);
    CHECK(removeDuplicatesList(list, intEquals) == 0, "consecutive dup list returns 0");
    CHECK(listLength(list) == 1, "consecutive duplicates collapse to 1 element");
    CHECK(*(int *)list->head->data == 1, "remaining element is 1");
    freeList(list);
}

static void test_remove_dup_list_order(void)
{
    printf("Test: removeDuplicatesList order preservation\n");

    GenericList *list = createList(sizeof(int));
    int vals[] = {1, 2, 1, 3, 2, 4};
    for (int i = 0; i < 6; i++) appendItem(list, &vals[i]);

    CHECK(removeDuplicatesList(list, intEquals) == 0, "returns 0 on success");
    CHECK(listLength(list) == 4, "6 -> 4 unique elements");
    CHECK(*(int *)list->head->data == 1, "first element preserved (1)");

    int i = 0;
    Node *cur = list->head;
    int expected[] = {1, 2, 3, 4};
    int ok = 1;
    while (cur != NULL && i < 4) {
        if (*(int *)cur->data != expected[i]) { ok = 0; break; }
        cur = cur->next;
        i++;
    }
    CHECK(ok && i == 4, "order of first occurrence preserved: 1,2,3,4");

    freeList(list);
}

static void test_remove_dup_list_strings(void)
{
    printf("Test: removeDuplicatesList strings\n");

    GenericList *list = createList(sizeof(char) * 32);
    char s[][32] = {"banana", "mama", "banana", "papa", "mama"};
    for (int i = 0; i < 5; i++) appendItem(list, s[i]);

    CHECK(removeDuplicatesList(list, stringEquals) == 0, "string dedup returns 0");
    CHECK(listLength(list) == 3, "5 -> 3 unique strings");

    const char *expected[] = {"banana", "mama", "papa"};
    int i = 0, ok = 1;
    Node *cur = list->head;
    while (cur != NULL && i < 3) {
        if (strcmp((char *)cur->data, expected[i]) != 0) { ok = 0; break; }
        cur = cur->next;
        i++;
    }
    CHECK(ok && i == 3, "strings deduplicated with order: banana,mama,papa");

    freeList(list);
}

static void test_remove_dup_list_structs(void)
{
    printf("Test: removeDuplicatesList structs\n");

    GenericList *list = createList(sizeof(Point));
    Point p[] = {{1, 2}, {3, 4}, {1, 2}, {5, 6}, {3, 4}};
    for (int i = 0; i < 5; i++) appendItem(list, &p[i]);

    CHECK(removeDuplicatesList(list, pointEquals) == 0, "struct dedup returns 0");
    CHECK(listLength(list) == 3, "5 -> 3 unique structs");

    Point *first = (Point *)list->head->data;
    CHECK(first->x == 1 && first->y == 2, "first struct preserved");

    freeList(list);
}

static void test_remove_dup_list_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: removeDuplicatesList (1,000,000 all-duplicate elements)\n");
    double t0 = now_seconds();

    /* All elements equal: the nested loops collapse after one pass, so this
       runs in roughly O(n) and finishes fast while still exercising 1M. */
    GenericList *list = createList(sizeof(int));
    Node *tail = NULL;
    for (unsigned int i = 0; i < N; i++) {
        Node *node = (Node *)malloc(sizeof(Node));
        node->data = malloc(sizeof(int));
        *(int *)node->data = 7;
        node->next = NULL;
        if (tail == NULL) list->head = node; else tail->next = node;
        tail = node;
    }

    CHECK(removeDuplicatesList(list, intEquals) == 0, "1M dedup returns 0");
    CHECK(listLength(list) == 1, "1M all-duplicate elements collapse to 1");
    CHECK(list->head != NULL && *(int *)list->head->data == 7, "remaining value is 7");

    freeList(list);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_remove_dup_list_unique_large(void)
{
    const unsigned int N = 20000u;
    printf("Test: removeDuplicatesList (20,000 unique elements)\n");
    double t0 = now_seconds();

    /* Unique-elements path is O(n^2), so kept at a moderate size to stay fast
       while still exercising a large no-dup list end to end. */
    GenericList *list = createList(sizeof(int));
    Node *tail = NULL;
    for (unsigned int i = 0; i < N; i++) {
        Node *node = (Node *)malloc(sizeof(Node));
        node->data = malloc(sizeof(int));
        *(int *)node->data = (int)i;
        node->next = NULL;
        if (tail == NULL) list->head = node; else tail->next = node;
        tail = node;
    }

    CHECK(removeDuplicatesList(list, intEquals) == 0, "large unique list returns 0");
    CHECK(listLength(list) == N, "large unique list keeps all N elements");

    freeList(list);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_remove_dup_vector_basic(void)
{
    printf("Test: removeDuplicatesVector basic\n");

    CHECK(removeDuplicatesVector(NULL, intEquals) == -1, "NULL vector returns -1");

    Vector *empty = createVector(sizeof(int));
    CHECK(removeDuplicatesVector(empty, intEquals) == -1, "empty vector returns -1");
    vectorFree(empty);

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){5});
    appendVectorItem(v, &(int){2});
    appendVectorItem(v, &(int){5});
    appendVectorItem(v, &(int){1});
    appendVectorItem(v, &(int){2});
    appendVectorItem(v, &(int){9});

    CHECK(removeDuplicatesVector(v, intEquals) == 0, "returns 0 on success");
    CHECK(v->size == 4, "6 -> 4 unique elements");

    int expected[] = {5, 2, 1, 9};
    int ok = 1;
    for (size_t i = 0; i < v->size; i++) {
        if (*(int *)getVectorItem(v, i) != expected[i]) { ok = 0; break; }
    }
    CHECK(ok, "order of first occurrence preserved: 5,2,1,9");
    CHECK(findVectorItem(v, &(int){5}, intEquals) == 0, "first 5 still at index 0");

    vectorFree(v);
}

static void test_remove_dup_vector_strings(void)
{
    printf("Test: removeDuplicatesVector strings\n");

    Vector *v = createVector(sizeof(char) * 32);
    char s[][32] = {"banana", "mama", "banana", "papa", "mama"};
    for (int i = 0; i < 5; i++) appendVectorItem(v, s[i]);

    CHECK(removeDuplicatesVector(v, stringEquals) == 0, "string dedup returns 0");
    CHECK(v->size == 3, "5 -> 3 unique strings");

    const char *expected[] = {"banana", "mama", "papa"};
    int ok = 1;
    for (size_t i = 0; i < v->size; i++) {
        if (strcmp((char *)getVectorItem(v, i), expected[i]) != 0) { ok = 0; break; }
    }
    CHECK(ok, "strings deduplicated with order: banana,mama,papa");

    vectorFree(v);
}

static void test_remove_dup_vector_single_and_unique(void)
{
    printf("Test: removeDuplicatesVector single & all-unique\n");

    Vector *v = createVector(sizeof(int));
    appendVectorItem(v, &(int){42});
    CHECK(removeDuplicatesVector(v, intEquals) == 0, "single element returns 0");
    CHECK(v->size == 1, "single element kept");
    vectorFree(v);

    v = createVector(sizeof(int));
    int vals[] = {1, 2, 3, 4};
    for (int i = 0; i < 4; i++) appendVectorItem(v, &vals[i]);
    CHECK(removeDuplicatesVector(v, intEquals) == 0, "all-unique returns 0");
    CHECK(v->size == 4, "all-unique vector unchanged");
    vectorFree(v);
}

static void test_remove_dup_vector_1m(void)
{
    const unsigned int N = 1000000u;
    printf("Test: removeDuplicatesVector (1,000,000 all-duplicate elements)\n");
    double t0 = now_seconds();

    /* All elements equal: write_idx stays 0, so the inner scan breaks on the
       very first comparison - overall O(n), keeping 1M instant. */
    Vector *v = createVector(sizeof(int));
    for (unsigned int i = 0; i < N; i++) {
        appendVectorItem(v, &(int){7});
    }

    CHECK(removeDuplicatesVector(v, intEquals) == 0, "1M dedup returns 0");
    CHECK(v->size == 1, "1M all-duplicate elements collapse to 1");
    CHECK(getVectorItem(v, 0) != NULL && *(int *)getVectorItem(v, 0) == 7, "remaining value is 7");

    vectorFree(v);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

static void test_remove_dup_vector_unique_large(void)
{
    const unsigned int N = 20000u;
    printf("Test: removeDuplicatesVector (20,000 unique elements)\n");
    double t0 = now_seconds();

    /* Unique-elements path is O(n^2); kept at a moderate size to stay fast
       while still exercising a large no-dup vector end to end. */
    Vector *v = createVector(sizeof(int));
    for (unsigned int i = 0; i < N; i++) {
        appendVectorItem(v, &(int){i});
    }

    CHECK(removeDuplicatesVector(v, intEquals) == 0, "large unique vector returns 0");
    CHECK(v->size == N, "large unique vector keeps all N elements");

    vectorFree(v);
    printf("  (%.2f s)\n", now_seconds() - t0);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== Base tasks tests ===\n\n");
    double start = now_seconds();

    test_find_max_student_basic();
    test_find_max_student_edges();
    test_find_max_student_1m();

    test_find_max_vector_basic();
    test_find_max_vector_types();
    test_find_max_vector_deep_copy();
    test_find_max_vector_1m();

    test_remove_dup_list_basic();
    test_remove_dup_list_order();
    test_remove_dup_list_strings();
    test_remove_dup_list_structs();
    test_remove_dup_list_1m();
    test_remove_dup_list_unique_large();

    test_remove_dup_vector_basic();
    test_remove_dup_vector_strings();
    test_remove_dup_vector_single_and_unique();
    test_remove_dup_vector_1m();
    test_remove_dup_vector_unique_large();

    printf("\n==========================\n");
    printf("Passed: %d, Failed: %d\n", tests_passed, tests_failed);
    printf("Total time: %.2f s\n", now_seconds() - start);
    return tests_failed == 0 ? 0 : 1;
}
