#ifndef KNAPSACK_H
#define KNAPSACK_H

#include <stddef.h>

// ============================================================
// Структуры данных
// ============================================================

typedef struct {
    int weight;
    int value;
    char name[64];
} Item;

typedef struct {
    Item *items;
    int size;
    int capacity;
} ItemList;

typedef struct {
    int totalValue;
    int totalWeight;
    int *taken;      // taken[i] = 1, если i-й предмет взят
    int takenCount;
} KnapsackResult;

// ============================================================
// Функции для работы со списком предметов
// ============================================================

// Создание пустого списка предметов
ItemList *createItemList(int capacity);

// Добавление предмета в список
int addItem(ItemList *list, int weight, int value, const char *name);

// Освобождение памяти списка
void freeItemList(ItemList *list);

// Печать списка предметов
void printItemList(ItemList *list);

// ============================================================
// Основные функции рюкзака
// ============================================================

// Решение задачи 0/1 рюкзака методом табуляции
// Возвращает результат с максимальной стоимостью и списком взятых предметов
// Возвращает NULL при ошибке
KnapsackResult *knapsack01(ItemList *items, int capacity);

// Оптимизированная версия с O(W) памяти
// Возвращает только максимальную стоимость (без списка взятых предметов)
int knapsack01Optimized(ItemList *items, int capacity);

// ============================================================
// Вспомогательные функции
// ============================================================

// Печать таблицы DP
// dp — двумерный массив размера (n+1) x (capacity+1)
void printDPTable(int **dp, int n, int capacity, ItemList *items);

// Восстановление решения по таблице DP
// Заполняет массив taken: taken[i] = 1, если предмет i взят
void reconstructSolution(int **dp, ItemList *items, int capacity, int *taken);

// Печать результата
void printKnapsackResult(KnapsackResult *result, ItemList *items);

// Освобождение памяти результата
void freeKnapsackResult(KnapsackResult *result);

#endif // KNAPSACK_H
