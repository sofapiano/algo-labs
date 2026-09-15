#ifndef FRACTIONAL_KNAPSACK_H
#define FRACTIONAL_KNAPSACK_H

#include <stddef.h>

typedef struct {
    double weight;
    double value;
    char name[64];
} Item;

typedef struct {
    Item *items;
    int size;
    int capacity;
} ItemList;

typedef struct {
    char name[64];
    double fraction;    // Доля предмета (0.0 до 1.0)
    double weight;      // Взятый вес
    double value;       // Взятая ценность
} TakenItem;

typedef struct {
    TakenItem *items;
    int size;
    double totalValue;
    double totalWeight;
} KnapsackResult;


ItemList *createItemList(int capacity);
int addItem(ItemList *list, double weight, double value, const char *name);
void freeItemList(ItemList *list);
void printItemList(ItemList *list);


double getValuePerWeight(Item *item);

void sortByValuePerWeight(ItemList *list);

KnapsackResult *fractionalKnapsack(ItemList *items, double capacity);
// Это чисто для сравнения наивная реализация)
double knapsack01Naive(ItemList *items, double capacity);

void printKnapsackResult(KnapsackResult *result);
void freeKnapsackResult(KnapsackResult *result);

#endif // FRACTIONAL_KNAPSACK_H
