#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ItemList *createItemList(int capacity)
{
    if (capacity < 1) capacity = 1;
    ItemList *list = (ItemList *)malloc(sizeof(ItemList));
    if (!list) return NULL;
    list->items = (Item *)malloc(capacity * sizeof(Item));
    if (!list->items) {
        free(list);
        return NULL;
    }
    list->size = 0;
    list->capacity = capacity;
    return list;
}

int addItem(ItemList *list, int weight, int value, const char *name)
{
    if (!list || !name) return 0;
    if (list->size >= list->capacity) {
        int newCapacity = list->capacity * 2;
        Item *tmp = (Item *)realloc(list->items, newCapacity * sizeof(Item));
        if (!tmp) return 0;
        list->items = tmp;
        list->capacity = newCapacity;
    }
    list->items[list->size].weight = weight;
    list->items[list->size].value = value;
    strncpy(list->items[list->size].name, name, 63);
    list->items[list->size].name[63] = '\0';
    list->size++;
    return 1;
}

void freeItemList(ItemList *list)
{
    if (!list) return;
    free(list->items);
    free(list);
}

void printItemList(ItemList *list)
{
    if (!list) return;
    printf("Предметы:\n");
    for (int i = 0; i < list->size; i++) {
        printf("  %-10s %3d кг, %6d$\n", list->items[i].name,
               list->items[i].weight, list->items[i].value);
    }
}

static int **allocateDPTable(int rows, int cols)
{
    if (rows < 1 || cols < 1) return NULL;
    int **dp = (int **)calloc(rows, sizeof(int *));
    if (!dp) return NULL;
    for (int i = 0; i < rows; i++) {
        dp[i] = (int *)calloc(cols, sizeof(int));
        if (!dp[i]) {
            for (int j = 0; j < i; j++) free(dp[j]);
            free(dp);
            return NULL;
        }
    }
    return dp;
}

static void freeDPTable(int **dp, int rows)
{
    if (!dp) return;
    for (int i = 0; i < rows; i++) free(dp[i]);
    free(dp);
}

static int maxInt(int a, int b)
{
    return a > b ? a : b;
}


void printDPTable(int **dp, int n, int capacity, ItemList *items)
{
    if (!dp) return;
    printf("Таблица DP:\n");
    printf("%-10s", "");
    for (int w = 0; w <= capacity; w++) printf("%5d", w);
    printf("\n");
    for (int i = 0; i <= n; i++) {
        if (i == 0) printf("%-10s", "(пусто)");
        else if (items) printf("%-10s", items->items[i - 1].name);
        else printf("%-10s", "");
        for (int w = 0; w <= capacity; w++) printf("%5d", dp[i][w]);
        printf("\n");
    }
    printf("\n");
}

void reconstructSolution(int **dp, ItemList *items, int capacity, int *taken)
{
    if (!dp || !items || !taken) return;
    int w = capacity;
    for (int i = items->size; i > 0; i--) {
        if (dp[i][w] != dp[i - 1][w]) {
            taken[i - 1] = 1;
            w -= items->items[i - 1].weight;
        }
    }
}

KnapsackResult *knapsack01(ItemList *items, int capacity)
{
    if (!items || capacity < 0) return NULL;
    int n = items->size;

    int **dp = allocateDPTable(n + 1, capacity + 1);
    if (!dp) return NULL;

    for (int i = 1; i <= n; i++) {
        for (int w = 0; w <= capacity; w++) {
            int without = dp[i - 1][w];
            int with = 0;
            if (w >= items->items[i - 1].weight) {
                with = dp[i - 1][w - items->items[i - 1].weight] + items->items[i - 1].value;
            }
            dp[i][w] = maxInt(without, with);
        }
    }

    int *taken = (int *)calloc(n, sizeof(int));
    if (!taken) {
        freeDPTable(dp, n + 1);
        return NULL;
    }
    reconstructSolution(dp, items, capacity, taken);

    KnapsackResult *result = (KnapsackResult *)malloc(sizeof(KnapsackResult));
    if (!result) {
        free(taken);
        freeDPTable(dp, n + 1);
        return NULL;
    }
    result->totalValue = dp[n][capacity];
    result->taken = taken;
    result->totalWeight = 0;
    result->takenCount = 0;
    for (int i = 0; i < n; i++) {
        if (taken[i]) {
            result->takenCount++;
            result->totalWeight += items->items[i].weight;
        }
    }

    freeDPTable(dp, n + 1);
    return result;
}

int knapsack01Optimized(ItemList *items, int capacity)
{
    if (!items || capacity < 0) return 0;
    int *dp = (int *)calloc(capacity + 1, sizeof(int));
    if (!dp) return 0;
    for (int i = 0; i < items->size; i++) {
        for (int w = capacity; w >= items->items[i].weight; w--) {
            dp[w] = maxInt(dp[w], dp[w - items->items[i].weight] + items->items[i].value);
        }
    }
    int result = dp[capacity];
    free(dp);
    return result;
}

void printKnapsackResult(KnapsackResult *result, ItemList *items)
{
    if (!result) return;
    printf("Взяты: ");
    int first = 1;
    for (int i = 0; i < items->size; i++) {
        if (result->taken[i]) {
            if (!first) printf(", ");
            printf("%s", items->items[i].name);
            first = 0;
        }
    }
    if (first) printf("(ничего)");
    printf("\n");
    printf("Вес: %d кг\n", result->totalWeight);
    printf("Стоимость: %d$\n", result->totalValue);
}

void freeKnapsackResult(KnapsackResult *result)
{
    if (!result) return;
    free(result->taken);
    free(result);
}