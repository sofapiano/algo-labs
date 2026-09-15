#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


ItemList *createItemList(int capacity)
{
}

int addItem(ItemList *list, double weight, double value, const char *name)
{
}

void freeItemList(ItemList *list)
{
}

void printItemList(ItemList *list)
{
}


double getValuePerWeight(Item *item)
{
}

static int compareByValuePerWeight(const void *a, const void *b)
{
}

void sortByValuePerWeight(ItemList *list)
{
}


KnapsackResult *fractionalKnapsack(ItemList *items, double capacity)
{
}


static double knapsack01Recursive(Item *items, int n, double capacity, int idx)
{
}

double knapsack01Naive(ItemList *items, double capacity)
{
}


void printKnapsackResult(KnapsackResult *result)
{
}

void freeKnapsackResult(KnapsackResult *result)
{
}
