#include "knapsack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


ItemList *createItemList(int capacity)
{
    if (capacity < 0)
        return NULL;
    if (capacity == 0)
        capacity = 1;

    ItemList *list = malloc(sizeof(ItemList));
    if (list == NULL)
        return NULL;

    list->items = malloc(sizeof(Item) * (size_t)capacity);
    if (list->items == NULL)
    {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = capacity;
    return list;
}

int addItem(ItemList *list, double weight, double value, const char *name)
{
    if (list == NULL || name == NULL || weight <= 0.0)
        return 0;
    if (list->size >= list->capacity)
        return 0;

    Item *item = &list->items[list->size];
    item->weight = weight;
    item->value = value;
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0';

    list->size++;
    return 1;
}

void freeItemList(ItemList *list)
{
    if (list == NULL)
        return;

    free(list->items);
    free(list);
}

void printItemList(ItemList *list)
{
    if (list == NULL)
    {
        printf("(NULL)\n");
        return;
    }

    for (int i = 0; i < list->size; i++)
    {
        const Item *item = &list->items[i];
        printf("  \"%s\"  %.2f кг, %.2f$  (%.2f $/кг)\n",
               item->name, item->weight, item->value,
               getValuePerWeight(item));
    }
}


double getValuePerWeight(const Item *item)
{
    if (item == NULL || item->weight <= 0.0)
        return 0.0;

    return item->value / item->weight;
}

static int compareByValuePerWeight(const void *a, const void *b)
{
    const Item *x = (const Item *)a;
    const Item *y = (const Item *)b;

    double vx = getValuePerWeight(x);
    double vy = getValuePerWeight(y);

    if (vx < vy)
        return 1;
    if (vx > vy)
        return -1;
    return 0;
}

void sortByValuePerWeight(ItemList *list)
{
    if (list == NULL || list->size <= 1)
        return;

    qsort(list->items, (size_t)list->size, sizeof(Item), compareByValuePerWeight);
}


KnapsackResult *fractionalKnapsack(ItemList *items, double capacity)
{
    if (items == NULL || capacity <= 0.0)
        return NULL;

    KnapsackResult *result = malloc(sizeof(KnapsackResult));
    if (result == NULL)
        return NULL;

    result->items = malloc(sizeof(TakenItem) * (size_t)items->size);
    if (result->items == NULL)
    {
        free(result);
        return NULL;
    }

    result->size = 0;
    result->totalValue = 0.0;
    result->totalWeight = 0.0;

    if (items->size == 0)
        return result;

    // Копируем предметы в буфер и сортируем его,
    // чтобы не менять порядок в исходном списке.
    Item *sorted = malloc(sizeof(Item) * (size_t)items->size);
    if (sorted == NULL)
    {
        free(result->items);
        free(result);
        return NULL;
    }

    memcpy(sorted, items->items, sizeof(Item) * (size_t)items->size);
    qsort(sorted, (size_t)items->size, sizeof(Item), compareByValuePerWeight);

    double remaining = capacity;

    for (int i = 0; i < items->size && remaining > 0.0; i++)
    {
        Item *item = &sorted[i];
        if (item->value <= 0.0)
            continue;

        TakenItem *taken = &result->items[result->size];

        strncpy(taken->name, item->name, sizeof(taken->name) - 1);
        taken->name[sizeof(taken->name) - 1] = '\0';

        if (item->weight <= remaining)
        {
            taken->fraction = 1.0;
            taken->weight = item->weight;
            taken->value = item->value;
            remaining -= item->weight;
        }
        else
        {
            taken->fraction = remaining / item->weight;
            taken->weight = remaining;
            taken->value = item->value * taken->fraction;
            remaining = 0.0;
        }

        result->totalWeight += taken->weight;
        result->totalValue += taken->value;
        result->size++;
    }

    free(sorted);
    return result;
}


static double knapsack01Recursive(Item *items, int n, double capacity, int idx)
{
    if (idx >= n)
        return 0.0;

    // Не берём предмет.
    double skip = knapsack01Recursive(items, n, capacity, idx + 1);

    // Берём предмет, если он помещается.
    double take = 0.0;
    if (items[idx].weight <= capacity)
    {
        take = items[idx].value +
               knapsack01Recursive(items, n, capacity - items[idx].weight, idx + 1);
    }

    return take > skip ? take : skip;
}

double knapsack01Naive(ItemList *items, double capacity)
{
    if (items == NULL || items->size == 0 || capacity <= 0.0)
        return 0.0;

    return knapsack01Recursive(items->items, items->size, capacity, 0);
}


void printKnapsackResult(KnapsackResult *result)
{
    if (result == NULL)
    {
        printf("(NULL)\n");
        return;
    }

    for (int i = 0; i < result->size; i++)
    {
        const TakenItem *taken = &result->items[i];
        printf("  \"%s\": %.1f%% (%.1f кг) → %.2f$\n",
               taken->name,
               taken->fraction * 100.0,
               taken->weight,
               taken->value);
    }

    printf("  ИТОГО: %.1f кг, %.2f$\n",
           result->totalWeight, result->totalValue);
}

void freeKnapsackResult(KnapsackResult *result)
{
    if (result == NULL)
        return;

    free(result->items);
    free(result);
}