#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed = 0;
static int failed = 0;

static void check(int condition, const char *name)
{
    if (condition) {
        printf("PASS: %s\n", name);
        passed++;
    } else {
        printf("FAIL: %s\n", name);
        failed++;
    }
}

static void checkResult(KnapsackResult *r, const char *name)
{
    check(r != NULL, name);
}

static void testEmptyList(void)
{
    ItemList *list = createItemList(4);
    check(list != NULL, "createItemList создаёт список");
    if (!list) return;

    KnapsackResult *r = knapsack01(list, 50);
    checkResult(r, "knapsack01 с пустым списком возвращает результат");
    if (r) {
        check(r->totalValue == 0, "пустой список: стоимость 0");
        check(r->totalWeight == 0, "пустой список: вес 0");
        check(r->takenCount == 0, "пустой список: ничего не взято");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 50) == 0, "optimized: пустой список даёт 0");
    freeItemList(list);
}

static void testZeroCapacity(void)
{
    ItemList *list = createItemList(4);
    addItem(list, 3, 1500, "Ноутбук");
    addItem(list, 5, 400, "Спальник");

    KnapsackResult *r = knapsack01(list, 0);
    checkResult(r, "knapsack01 при вместимости 0");
    if (r) {
        check(r->totalValue == 0 && r->totalWeight == 0, "вместимость 0: ничего не влезает");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 0) == 0, "optimized: вместимость 0 даёт 0");
    freeItemList(list);
}

static void testTooHeavyItem(void)
{
    ItemList *list = createItemList(4);
    addItem(list, 100, 500, "Сейф");

    KnapsackResult *r = knapsack01(list, 50);
    checkResult(r, "knapsack01 с тяжёлым предметом");
    if (r) {
        check(r->totalValue == 0, "предмет тяжелее рюкзака не берётся");
        check(r->totalWeight == 0 && r->takenCount == 0, "тяжёлый предмет: вес 0, ничего не взято");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 50) == 0, "optimized: тяжёлый предмет даёт 0");
    freeItemList(list);
}

static void testSingleItem(void)
{
    ItemList *list = createItemList(4);
    addItem(list, 10, 100, "Мяч");

    KnapsackResult *r = knapsack01(list, 50);
    checkResult(r, "knapsack01 с одним предметом");
    if (r) {
        check(r->totalValue == 100, "один влезающий предмет");
        check(r->totalWeight == 10 && r->takenCount == 1, "один влезающий предмет: вес и количество");
        check(r->taken[0] == 1, "предмет помечен как взятый");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 50) == 100, "optimized: один предмет");
    freeItemList(list);
}

static void testReadmeExample(void)
{
    ItemList *list = createItemList(8);
    addItem(list, 3, 1500, "Ноутбук");
    addItem(list, 1, 2000, "Камера");
    addItem(list, 15, 300, "Книги");
    addItem(list, 20, 200, "Гантели");
    addItem(list, 10, 500, "Палатка");
    addItem(list, 5, 400, "Спальник");

    KnapsackResult *r = knapsack01(list, 50);
    checkResult(r, "knapsack01 на примере из README");
    if (r) {
        check(r->totalValue == 4700, "пример из README: стоимость 4700");
        check(r->totalWeight == 34, "пример из README: вес 34");
        check(r->takenCount == 5, "пример из README: 5 предметов");
        check(r->taken[0] == 1 && r->taken[1] == 1, "пример: взяты Ноутбук и Камера");
        check(r->taken[2] == 1 && r->taken[3] == 0, "пример: взяты Книги, Гантели не взяты");
        check(r->taken[4] == 1 && r->taken[5] == 1, "пример: взяты Палатка и Спальник");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 50) == 4700, "optimized: 4700");
    freeItemList(list);
}

static void testGreedyCounterexample(void)
{
    ItemList *list = createItemList(4);
    addItem(list, 10, 60, "A");
    addItem(list, 20, 100, "B");
    addItem(list, 30, 120, "C");

    KnapsackResult *r = knapsack01(list, 50);
    checkResult(r, "knapsack01 на контрпримере жадному алгоритму");
    if (r) {
        check(r->totalValue == 220, "контрпример жадному: оптимально 220 (B + C)");
        check(r->totalWeight == 50, "контрпример: вес 50");
        check(r->taken[0] == 0 && r->taken[1] == 1 && r->taken[2] == 1, "контрпример: взяты B и C");
        freeKnapsackResult(r);
    }
    check(knapsack01Optimized(list, 50) == 220, "optimized: 220");
    freeItemList(list);
}

static void runDemo(void)
{
    ItemList *list = createItemList(8);
    addItem(list, 3, 1500, "Ноутбук");
    addItem(list, 1, 2000, "Камера");
    addItem(list, 15, 300, "Книги");
    addItem(list, 20, 200, "Гантели");
    addItem(list, 10, 500, "Палатка");
    addItem(list, 5, 400, "Спальник");

    int capacity = 50;
    printf("Грузоподъёмность: %d кг\n\n", capacity);
    printItemList(list);

    int n = list->size;
    int **dp = (int **)calloc(n + 1, sizeof(int *));
    for (int i = 0; i <= n; i++) dp[i] = (int *)calloc(capacity + 1, sizeof(int));
    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            int without = dp[i - 1][w];
            int with = 0;
            if (w >= list->items[i - 1].weight) {
                with = dp[i - 1][w - list->items[i - 1].weight] + list->items[i - 1].value;
            }
            dp[i][w] = without > with ? without : with;
        }
    }
    printf("\n");
    printDPTable(dp, n, capacity, list);
    for (int i = 0; i <= n; i++) free(dp[i]);
    free(dp);

    KnapsackResult *r = knapsack01(list, capacity);
    printKnapsackResult(r, list);
    printf("\n");

    printf("Оптимизированная версия (O(W) памяти): %d$\n\n", knapsack01Optimized(list, capacity));
    freeKnapsackResult(r);
    freeItemList(list);

    printf("Контрпример против жадного алгоритма:\n");
    ItemList *g = createItemList(4);
    addItem(g, 10, 60, "A");
    addItem(g, 20, 100, "B");
    addItem(g, 30, 120, "C");
    printItemList(g);
    printf("Жадный (по ценности/кг):        A + B = 160$ (30 кг)\n");
    printf("Оптимальный (динамическое ДП):  ");
    KnapsackResult *gr = knapsack01(g, 50);
    printKnapsackResult(gr, g);
    freeKnapsackResult(gr);
    freeItemList(g);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        runDemo();
        return 0;
    }

    testEmptyList();
    testZeroCapacity();
    testTooHeavyItem();
    testSingleItem();
    testReadmeExample();
    testGreedyCounterexample();

    printf("\nИтог: %d пройдено, %d провалено\n", passed, failed);
    return failed == 0 ? 0 : 1;
}