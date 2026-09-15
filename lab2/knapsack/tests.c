#include "knapsack.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

static int tests_passed = 0;
static int tests_failed = 0;

static void check(int condition, const char *description)
{
    if (condition)
    {
        tests_passed++;
        printf("  OK  %s\n", description);
    }
    else
    {
        tests_failed++;
        printf("  FAIL %s\n", description);
    }
}

static int near(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

static int compareByDensity(const void *a, const void *b)
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

/* ============================================================ */
/*  Создание и заполнение списка                                */
/* ============================================================ */
static void test_create_and_add(void)
{
    printf("\n--- test_create_and_add ---\n");

    ItemList *list = createItemList(3);
    check(list != NULL,        "createItemList(3) не NULL");
    check(list->size == 0,     "size == 0 после создания");
    check(list->capacity == 3, "capacity == 3");

    check(addItem(list, 10.0, 60.0, "A") == 1, "addItem A");
    check(addItem(list, 20.0, 100.0, "B") == 1, "addItem B");
    check(addItem(list, 30.0, 120.0, "C") == 1, "addItem C");

    check(list->size == 3, "size == 3 после трёх добавлений");
    check(addItem(list, 5.0, 50.0, "D") == 0,
          "addItem за пределы capacity возвращает 0");

    check(near(list->items[0].weight, 10.0), "A.weight == 10");
    check(near(list->items[0].value, 60.0),  "A.value == 60");
    check(strcmp(list->items[1].name, "B") == 0, "имя B хранится корректно");

    check(addItem(NULL, 1.0, 2.0, "X") == 0, "addItem(NULL) == 0");
    check(addItem(list, -1.0, 5.0, "Neg") == 0,
          "addItem с неположительным весом == 0");

    freeItemList(list);
    printf("  OK  freeItemList без крашей\n");
}

/* ============================================================ */
/*  Удельная стоимость и сортировка                             */
/* ============================================================ */
static void test_value_per_weight(void)
{
    printf("\n--- test_value_per_weight ---\n");

    Item gold = {10.0, 600.0, "Gold"};
    Item bronze = {30.0, 300.0, "Bronze"};

    check(near(getValuePerWeight(&gold), 60.0), "Gold: 600/10 = 60 $/кг");
    check(near(getValuePerWeight(&bronze), 10.0), "Bronze: 300/30 = 10 $/кг");

    Item zero = {0.0, 5.0, "Zero"};
    check(near(getValuePerWeight(&zero), 0.0),
          "нулевой вес → удельная стоимость 0 (без деления на ноль)");
    check(near(getValuePerWeight(NULL), 0.0), "getValuePerWeight(NULL) == 0");
}

static void test_sort(void)
{
    printf("\n--- test_sort ---\n");

    ItemList *list = createItemList(3);
    addItem(list, 30.0, 300.0, "Bronze");   // 10 $/кг
    addItem(list, 10.0, 600.0, "Gold");     // 60 $/кг
    addItem(list, 20.0, 400.0, "Silver");   // 20 $/кг

    sortByValuePerWeight(list);

    check(strcmp(list->items[0].name, "Gold") == 0, "первый — Gold (60 $/кг)");
    check(strcmp(list->items[1].name, "Silver") == 0, "второй — Silver (20 $/кг)");
    check(strcmp(list->items[2].name, "Bronze") == 0, "третий — Bronze (10 $/кг)");

    sortByValuePerWeight(list);  // повторная сортировка не ломает данные
    check(list->size == 3, "размер не изменился после повторной сортировки");

    freeItemList(list);
}

/* ============================================================ */
/*  Дробный рюкзак — основной пример (A, B, C, 50 кг)          */
/* ============================================================ */
static void test_fractional_basic(void)
{
    printf("\n--- test_fractional_basic ---\n");

    ItemList *items = createItemList(3);
    addItem(items, 30.0, 120.0, "C");
    addItem(items, 20.0, 100.0, "B");
    addItem(items, 10.0, 60.0, "A");

    KnapsackResult *result = fractionalKnapsack(items, 50.0);

    check(result != NULL, "fractionalKnapsack не NULL");
    check(result->size == 3, "взято 3 предмета");
    check(strcmp(result->items[0].name, "A") == 0, "1-й предмет — A");
    check(near(result->items[0].fraction, 1.0), "A взят целиком (100%)");
    check(strcmp(result->items[1].name, "B") == 0, "2-й предмет — B");
    check(near(result->items[1].fraction, 1.0), "B взят целиком (100%)");
    check(strcmp(result->items[2].name, "C") == 0, "3-й предмет — C");
    check(near(result->items[2].fraction, 20.0 / 30.0),
          "C взят на 66.7%");
    check(near(result->totalWeight, 50.0), "итоговый вес == 50 кг");
    check(near(result->totalValue, 240.0), "итоговая стоимость == 240$");

    // выбираем по стоимости, но золото в конце — сортировка внутри функции
    check(strcmp(items->items[0].name, "C") == 0,
          "исходный список не мутирован (C на месте)");

    freeKnapsackResult(result);
    freeItemList(items);
}

/* ============================================================ */
/*  Дробный рюкзак — пример из README (Золото/Серебро/Бронза)  */
/* ============================================================ */
static void test_fractional_readme(void)
{
    printf("\n--- test_fractional_readme ---\n");

    ItemList *items = createItemList(3);
    addItem(items, 10.0, 600.0, "Золото");
    addItem(items, 20.0, 400.0, "Серебро");
    addItem(items, 30.0, 300.0, "Бронза");

    KnapsackResult *result = fractionalKnapsack(items, 50.0);

    check(result->size == 3, "взято 3 предмета");
    check(strcmp(result->items[0].name, "Золото") == 0, "1-й — Золото");
    check(near(result->items[1].fraction, 1.0), "Серебро целиком");
    check(near(result->items[2].fraction, 2.0 / 3.0), "Бронза на 66.7%");
    check(near(result->totalValue, 1200.0), "итог == 1200$");
    check(near(result->totalWeight, 50.0), "итог == 50 кг");

    freeKnapsackResult(result);
    freeItemList(items);
}

/* ============================================================ */
/*  Крайние случаи                                              */
/* ============================================================ */
static void test_edge_cases(void)
{
    printf("\n--- test_edge_cases ---\n");

    /* Рюкзак больше суммарного веса — берём всё */
    ItemList *light = createItemList(2);
    addItem(light, 1.0, 10.0, "X");
    addItem(light, 2.0, 10.0, "Y");

    KnapsackResult *result = fractionalKnapsack(light, 100.0);
    check(result->size == 2, "вместимость больше веса — берём всё");
    check(near(result->totalWeight, 3.0), "вес всего груза == 3 кг");
    check(near(result->totalValue, 20.0), "стоимость всего груза == 20$");
    freeKnapsackResult(result);

    /* Нулевая вместимость */
    result = fractionalKnapsack(light, 0.0);
    check(result == NULL, "вместимость 0 → NULL");
    freeItemList(light);

    /* Пустой список */
    ItemList *empty = createItemList(3);
    result = fractionalKnapsack(empty, 50.0);
    check(result != NULL, "пустой список → не NULL");
    check(result->size == 0, "из пустого списка ничего не взято");
    check(near(result->totalValue, 0.0), "стоимость == 0");
    freeKnapsackResult(result);
    freeItemList(empty);

    /* Один предмет тяжелее рюкзака — берём часть */
    ItemList *heavy = createItemList(1);
    addItem(heavy, 10.0, 100.0, "Big");

    result = fractionalKnapsack(heavy, 3.0);
    check(result->size == 1, "тяжёлый предмет: взята часть");
    check(near(result->items[0].fraction, 0.3), "доля == 0.3");
    check(near(result->totalValue, 30.0), "стоимость == 30$");
    freeKnapsackResult(result);
    freeItemList(heavy);

    /* Бесполезные предметы (value == 0) пропускаются */
    ItemList *junk = createItemList(2);
    addItem(junk, 5.0, 0.0, "Junk");
    addItem(junk, 5.0, 100.0, "Good");

    result = fractionalKnapsack(junk, 50.0);
    check(result->size == 1, "бесполезный предмет пропущен");
    check(strcmp(result->items[0].name, "Good") == 0, "взят полезный предмет");
    freeKnapsackResult(result);
    freeItemList(junk);

    check(fractionalKnapsack(NULL, 50.0) == NULL, "fractionalKnapsack(NULL) == NULL");
}

/* ============================================================ */
/*  Наивный 0/1 рюкзак                                          */
/* ============================================================ */
static void test_knapsack01(void)
{
    printf("\n--- test_knapsack01 ---\n");

    ItemList *items = createItemList(3);
    addItem(items, 10.0, 60.0, "A");
    addItem(items, 20.0, 100.0, "B");
    addItem(items, 30.0, 120.0, "C");

    // Оптимум: B + C = 220.
    check(near(knapsack01Naive(items, 50.0), 220.0),
          "0/1: B + C == 220$ (оптимум)");

    // Вместимость 30: оптимум A + B = 160 (A+lегче, B).
    check(near(knapsack01Naive(items, 30.0), 160.0),
          "0/1 (30 кг): A + B == 160$");

    // Только A помещается в рюкзак 10 кг.
    check(near(knapsack01Naive(items, 10.0), 60.0),
          "0/1 (10 кг): A == 60$");

    check(near(knapsack01Naive(NULL, 50.0), 0.0), "knapsack01Naive(NULL) == 0");
    check(near(knapsack01Naive(items, 0.0), 0.0), "knapsack01Naive(capacity=0) == 0");

    freeItemList(items);
}

/* ============================================================ */
/*  NULL-безопасность списка                                    */
/* ============================================================ */
static void test_null_safety(void)
{
    printf("\n--- test_null_safety ---\n");

    check(createItemList(-5) == NULL, "createItemList(-5) == NULL");
    check(createItemList(0) != NULL, "createItemList(0) корректен");

    ItemList *list = createItemList(1);
    printItemList(NULL);
    printItemList(list);
    freeItemList(list);
    check(1, "printItemList не падает на NULL");

    freeKnapsackResult(NULL);
    check(1, "freeKnapsackResult(NULL) не падает");
}

/* ============================================================ */
/*  Демонстрация (--demo)                                       */
/* ============================================================ */
static void run_demo(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Демонстрация — Fractional Knapsack (жадный алгоритм)  ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* Пример 1: из README */
    printf("\n▸ Пример 1 (Золото/Серебро/Бронза, 50 кг):\n\n");

    ItemList *items = createItemList(3);
    addItem(items, 10.0, 600.0, "Золото");
    addItem(items, 20.0, 400.0, "Серебро");
    addItem(items, 30.0, 300.0, "Бронза");

    printf("  Предметы:\n");
    printItemList(items);

    KnapsackResult *result = fractionalKnapsack(items, 50.0);
    printf("\n  Жадный выбор:\n");
    printKnapsackResult(result);
    freeKnapsackResult(result);
    freeItemList(items);

    /* Пример 2: сравнение 0/1 и дробного */
    printf("\n▸ Пример 2 (жадный проигрывает для 0/1), 50 кг:\n\n");

    ItemList *compare = createItemList(3);
    addItem(compare, 10.0, 60.0, "A");   // 6 $/кг
    addItem(compare, 20.0, 100.0, "B");  // 5 $/кг
    addItem(compare, 30.0, 120.0, "C");  // 4 $/кг

    result = fractionalKnapsack(compare, 50.0);
    printf("  Жадный (дробный): ");
    printKnapsackResult(result);
    freeKnapsackResult(result);

    // 0/1 жадный: сортируем по плотности, берём целиком пока влезает.
    double greedy01 = 0.0;
    Item *sorted = malloc(sizeof(Item) * (size_t)compare->size);
    memcpy(sorted, compare->items, sizeof(Item) * (size_t)compare->size);
    qsort(sorted, (size_t)compare->size, sizeof(Item), compareByDensity);
    {
        double rest = 50.0;
        for (int i = 0; i < compare->size; i++)
        {
            if (sorted[i].weight <= rest)
            {
                greedy01 += sorted[i].value;
                rest -= sorted[i].weight;
            }
        }
    }
    free(sorted);

    double optimal01 = knapsack01Naive(compare, 50.0);
    printf("\n  0/1 жадный  (50 кг): берёт A + B = %.2f$ (неоптимально)\n",
           greedy01);
    printf("  0/1 оптимум (50 кг): B + C = %.2f$  ← жадный проиграл!\n",
           optimal01);
    freeItemList(compare);

    printf("\n  Вывод: жадный даёт оптимум для дробного рюкзака,\n");
    printf("  но НЕ для 0/1 варианта.\n");
}

/* ============================================================ */
/*  main                                                        */
/* ============================================================ */
int main(int argc, char *argv[])
{
    printf("=== Fractional Knapsack — тесты ===\n");

    test_create_and_add();
    test_value_per_weight();
    test_sort();
    test_fractional_basic();
    test_fractional_readme();
    test_edge_cases();
    test_knapsack01();
    test_null_safety();

    printf("\n--- Итого: пройдено %d, провалено %d ---\n",
           tests_passed, tests_failed);

    if (argc > 1 && strcmp(argv[1], "--demo") == 0)
        run_demo();

    return tests_failed == 0 ? 0 : 1;
}