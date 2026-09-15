#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


ItemList *createItemList(int capacity)
{ 
    // TODO реализовать
}

int addItem(ItemList *list, int weight, int value, const char *name)
{
    // TODO реализовать
}

void freeItemList(ItemList *list)
{
    // TODO реализовать
}

void printItemList(ItemList *list)
{
    // TODO реализовать
}

static int **allocateDPTable(int rows, int cols)
{
    // TODO реализовать
}

static void freeDPTable(int **dp, int rows)
{
    // TODO реализовать
}

static int maxInt(int a, int b)
{
    // TODO реализовать
}


void printDPTable(int **dp, int n, int capacity, ItemList *items)
{
    // TODO реализовать
}

void reconstructSolution(int **dp, ItemList *items, int capacity, int *taken)
{
    // TODO реализовать
}

KnapsackResult *knapsack01(ItemList *items, int capacity)
{
    // TODO реализовать
}

int knapsack01Optimized(ItemList *items, int capacity)
{
    // TODO реализовать
}

void printKnapsackResult(KnapsackResult *result, ItemList *items)
{
    // TODO реализовать
}

void freeKnapsackResult(KnapsackResult *result)
{
    // TODO реализовать
}
