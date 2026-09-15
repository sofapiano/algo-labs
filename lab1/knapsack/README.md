# Задача 1: Рюкзак 0/1 (0/1 Knapsack)

## Описание

Классическая задача динамического программирования: дано N предметов с весом и ценностью, рюкзак вместимостью W. Найти максимальную ценность предметов, которые можно унести, при условии что каждый предмет либо берётся целиком, либо не берётся вообще.

## Рекуррентное соотношение

```
dp[i][w] = максимальная ценность при использовании первых i предметов
           и вместимости w

dp[i][w] = max(
    dp[i-1][w],                           // не берём i-й предмет
    dp[i-1][w - weight[i]] + value[i]     // берём i-й предмет (если влезает)
)

База: dp[0][w] = 0 для всех w (нет предметов — нет ценности)
```

## Файлы

```
knapsack/
├── knapsack.h    # Заголовочный файл с объявлениями
├── knapsack.c    # Реализация алгоритма
├── tests.c       # Тесты
└── README.md     # Этот файл
```

## Сборка и запуск

### Запуск тестов

```bash
gcc -Wall -Wextra -std=c11 -o tests knapsack.c tests.c
./tests
```

### Демонстрация работы

```bash
./tests --demo
```

Выводит пример работы алгоритма с печатью списка предметов и результата.

## API

### Структуры

```c
// Предмет
typedef struct {
    int weight;      // Вес
    int value;       // Ценность
    char name[64];   // Название
} Item;

// Список предметов
typedef struct {
    Item *items;
    int size;
    int capacity;
} ItemList;

// Результат
typedef struct {
    int totalValue;   // Общая ценность
    int totalWeight;  // Общий вес
    int *taken;       // taken[i] = 1, если i-й предмет взят
    int takenCount;   // Количество взятых предметов
} KnapsackResult;
```

### Основные функции

```c
// Решение методом табуляции — O(N*W) времени, O(N*W) памяти
KnapsackResult *knapsack01(ItemList *items, int capacity);

// Оптимизированная версия — O(N*W) времени, O(W) памяти
// Возвращает только максимальную ценность (без списка взятых предметов)
int knapsack01Optimized(ItemList *items, int capacity);
```

### Вспомогательные функции

```c
// Работа со списком предметов
ItemList *createItemList(int capacity);
int addItem(ItemList *list, int weight, int value, const char *name);
void freeItemList(ItemList *list);
void printItemList(ItemList *list);

// Работа с результатом
void printKnapsackResult(KnapsackResult *result, ItemList *items);
void freeKnapsackResult(KnapsackResult *result);

// Визуализация таблицы DP
void printDPTable(int **dp, int n, int capacity, ItemList *items);
```

## Пример использования

```c
#include "knapsack.h"
#include <stdio.h>

int main()
{
    // Создаём список предметов
    ItemList *items = createItemList(8);
    addItem(items, 3, 1500, "Ноутбук");
    addItem(items, 1, 2000, "Камера");
    addItem(items, 15, 300, "Книги");
    addItem(items, 20, 200, "Гантели");
    addItem(items, 10, 500, "Палатка");
    addItem(items, 5, 400, "Спальник");

    // Решаем задачу
    KnapsackResult *result = knapsack01(items, 50);

    // Выводим результат
    printKnapsackResult(result, items);

    // Освобождаем память
    freeKnapsackResult(result);
    freeItemList(items);

    return 0;
}
```

## Сложность

| Версия | Время | Память |
|--------|-------|--------|
| `knapsack01` | O(N × W) | O(N × W) |
| `knapsack01Optimized` | O(N × W) | O(W) |

где N — количество предметов, W — вместимость рюкзака.

## Почему жадный алгоритм не работает?

Жадный подход: сортируем по ценность/вес и берём самые "выгодные" предметы.

**Контрпример:**
```
Предметы: A(10кг, 60$), B(20кг, 100$), C(30кг, 120$)
Вместимость: 50кг

По соотношению ценность/вес:
  A: 6.0 $/кг (лучшее)
  B: 5.0 $/кг
  C: 4.0 $/кг (худшее)

Жадный алгоритм: A + B = 160$ (вес 30кг, место осталось!)
Оптимальный:     B + C = 220$ (вес 50кг)
```

Жадный проигрывает, потому что не умеет "заглядывать вперёд" — он не знает, что оставив место, можно было бы взять более тяжёлый, но ценный предмет.
