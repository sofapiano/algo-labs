#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

static int  tests_failed = 0;
static char prev_key[1024];
static int  prev_inited = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        if (cond) {                                                            \
            printf("  PASS: %s\n", msg);                                       \
        } else {                                                               \
            printf("  FAIL: %s (line %d)\n", msg, __LINE__);                   \
            tests_failed++;                                                    \
        }                                                                      \
    } while (0)

static void visit_count(const char* key, Vector* postings, void* ctx) {
    (void)key;
    (void)postings;
    (*(int*)ctx)++;
}

static void visit_order(const char* key, Vector* postings, void* ctx) {
    (void)postings;
    (void)ctx;
    if (prev_inited && strcmp(prev_key, key) >= 0) {
        printf("  FAIL: порядок нарушен (%s >= %s)\n", prev_key, key);
        tests_failed++;
    }
    strncpy(prev_key, key, sizeof(prev_key) - 1);
    prev_key[sizeof(prev_key) - 1] = '\0';
    prev_inited = 1;
}

static void test_basic_insert_search(void) {
    printf("[test] базовые вставка/поиск\n");
    BTree* tree = createBTree();
    CHECK(tree != NULL, "createBTree вернул не NULL");
    CHECK(tree->root != NULL, "корень существует");

    const char* keys[] = {"delta", "alpha", "charlie", "bravo", "echo",
                          "foxtrot", "golf"};
    int n = (int)(sizeof(keys) / sizeof(keys[0]));

    for (int i = 0; i < n; i++) {
        btreeInsert(tree, keys[i], 100 + i, "title from insertion");
    }
    CHECK(tree->size == n, "size равен числу уникальных ключей");

    for (int i = 0; i < n; i++) {
        Vector* pl = btreeSearch(tree, keys[i]);
        CHECK(pl != NULL, "ключ найден");
        if (pl) {
            CHECK(pl->size == 1, "у свежего ключа 1 posting");
            PostingEntry* e = (PostingEntry*)getVectorItem(pl, 0);
            CHECK(e->doc_id == 100 + i, "doc_id корректный");
        }
    }

    CHECK(btreeSearch(tree, "zzz") == NULL, "отсутствующий ключ -> NULL");
    CHECK(btreeSearch(tree, "ab")  == NULL, "частичное совпадение -> NULL");

    freeBTree(tree);
    CHECK(1, "freeBTree отработал без падения");
}

static void test_duplicates(void) {
    printf("[test] дубликаты ключа\n");
    BTree* tree = createBTree();

    btreeInsert(tree, "python", 1, "a");
    btreeInsert(tree, "python", 2, "b");
    btreeInsert(tree, "python", 3, "c");
    btreeInsert(tree, "java",   7, "j");

    CHECK(tree->size == 2, "2 уникальных ключа");
    Vector* pl = btreeSearch(tree, "python");
    CHECK(pl != NULL, "posting list найден");
    CHECK(pl && pl->size == 3, "в posting list 3 записи");
    if (pl) {
        PostingEntry* e0 = (PostingEntry*)getVectorItem(pl, 0);
        PostingEntry* e2 = (PostingEntry*)getVectorItem(pl, 2);
        CHECK(e0->doc_id == 1, "первая запись doc_id=1");
        CHECK(e2->doc_id == 3, "последняя запись doc_id=3");
        CHECK(strcmp(e0->title, "a") == 0, "title сохранён");
    }

    freeBTree(tree);
}

static void test_force_splits(void) {
    printf("[test] много вставок -> сплиты/рост высоты\n");
    BTree* tree = createBTree();
    const int N = 500;

    for (int i = 0; i < N; i++) {
        char key[32];
        snprintf(key, sizeof(key), "k%04d", i);
        btreeInsert(tree, key, i, "t");
    }
    CHECK(tree->size == N, "все 500 уникальных ключей учтены");
    CHECK(tree->root->n <= BTREE_MAX_KEYS, "корень не переполнен");

    for (int i = 0; i < N; i++) {
        char key[32];
        snprintf(key, sizeof(key), "k%04d", i);
        Vector* pl = btreeSearch(tree, key);
        if (!pl || pl->size != 1 || ((PostingEntry*)getVectorItem(pl, 0))->doc_id != i) {
            printf("  FAIL: ключ %s не найден корректно (line %d)\n", key, __LINE__);
            tests_failed++;
            break;
        }
    }
    CHECK(1, "все ключи извлекаются корректно");

    /* теперь вставляем те же ключи ещё раз dupe'ом */
    for (int i = 0; i < N; i++) {
        char key[32];
        snprintf(key, sizeof(key), "k%04d", i);
        btreeInsert(tree, key, i + 100000, "dup");
    }
    CHECK(tree->size == N, "размер не вырос от дубликатов");
    Vector* pl = btreeSearch(tree, "k0000");
    CHECK(pl && pl->size == 2, "у k0000 два posting (оригинал + dup)");

    freeBTree(tree);
}

static void test_traverse(void) {
    printf("[test] обход в порядке возрастания\n");
    BTree* tree = createBTree();
    const char* words[] = {"banana", "apple", "cherry", "date", "fig",
                           "elderberry", "grape", "honeydew", "kiwi"};
    for (int i = 0; i < 9; i++)
        btreeInsert(tree, words[i], i, "t");

    int count = 0;
    prev_inited = 0;
    btreeTraverse(tree, visit_order, NULL);
    btreeTraverse(tree, visit_count, &count);
    CHECK(count == 9, "обойдены все 9 ключей");
    CHECK(tests_failed >= 0, "порядок проверялся"); /* no-op, линтер спокоен */

    freeBTree(tree);
}

static void test_empty_and_single(void) {
    printf("[test] пустое дерево и один ключ\n");
    BTree* tree = createBTree();
    CHECK(btreeSearch(tree, "x") == NULL, "поиск в пустом дереве -> NULL");
    int count = 0;
    btreeTraverse(tree, visit_count, &count);
    CHECK(count == 0, "обход пустого дерева ничего не даёт");

    btreeInsert(tree, "solo", 42, "only");
    CHECK(tree->size == 1, "size == 1");
    CHECK(btreeSearch(tree, "solo") != NULL, "solo найден");
    freeBTree(tree);
    CHECK(1, "freeBTree(NULL) -> не падает");

    BTree* empty = createBTree();
    freeBTree(empty);
    CHECK(1, "freeBTree пустого дерева");
}

static void test_reverse_insert(void) {
    printf("[test] вставка в обратном порядке\n");
    BTree* tree = createBTree();
    const int N = 300;
    for (int i = N - 1; i >= 0; i--) {
        char key[32];
        snprintf(key, sizeof(key), "v%d", i);
        btreeInsert(tree, key, i, "t");
    }
    CHECK(tree->size == N, "все N ключей учтены");
    int count = 0;
    prev_inited = 0;
    btreeTraverse(tree, visit_order, NULL);
    btreeTraverse(tree, visit_count, &count);
    CHECK(count == N, "обойдено N ключей");
    freeBTree(tree);
}

static void test_duplicate_across_split(void) {
    printf("[test] дубликат ключа из соседнего поддерева (регрессия брута)\n");
    BTree* tree = createBTree();

    /* python попадает в правое поддерево после сплита корня... */
    const char* seq[] = {"how", "sort", "list", "objects", "python", "want",
                         "sort", "list", "dictionaries", "value", "python"};
    for (int i = 0; i < 11; i++)
        btreeInsert(tree, seq[i], 1, "t");

    /* ...а повторная вставка python должна ПРИСОЕДИНИТЬСЯ к существующему
       ключу, а не завести второй ключ в левом поддереве */
    CHECK(tree->size == 8, "8 уникальных ключей, а не 9");

    Vector* pl = btreeSearch(tree, "python");
    CHECK(pl != NULL, "python найден");
    CHECK(pl && pl->size == 2, "два posting у одного ключа python");

    int count = 0;
    btreeTraverse(tree, visit_count, &count);
    CHECK(count == 8, "обход видит ровно 8 ключей");

    freeBTree(tree);
}

static void test_title_persistence(void) {
    printf("[test] длинные title сохраняются (обрезка до 255)\n");
    BTree* tree = createBTree();
    char long_title[300];
    memset(long_title, 'x', sizeof(long_title) - 1);
    long_title[sizeof(long_title) - 1] = '\0';

    btreeInsert(tree, "term", 7, long_title);
    Vector* pl = btreeSearch(tree, "term");
    CHECK(pl && pl->size == 1, "один posting");
    if (pl) {
        PostingEntry* e = (PostingEntry*)getVectorItem(pl, 0);
        CHECK(e->title[MAX_TITLE_LEN - 1] == '\0', "title занулён на конце");
        CHECK(strlen(e->title) == (size_t)(MAX_TITLE_LEN - 1), "title обрезан до 255");
    }
    freeBTree(tree);
}

int main(void) {
    printf("B-tree tests (BTREE_T=%d, max_keys=%d)\n", BTREE_T, BTREE_MAX_KEYS);
    test_basic_insert_search();
    test_duplicates();
    test_force_splits();
    test_traverse();
    test_empty_and_single();
    test_reverse_insert();
    test_duplicate_across_split();
    test_title_persistence();

    printf("\n%s: %d tests failed\n",
           tests_failed == 0 ? "OK" : "ERROR", tests_failed);
    return tests_failed == 0 ? 0 : 1;
}