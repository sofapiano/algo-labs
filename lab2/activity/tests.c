#include "activity.h"
#include <stdio.h>
#include <string.h>

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

/* ============================================================ */
/*  Создание и заполнение списка                                */
/* ============================================================ */
static void test_create_and_add(void)
{
    printf("\n--- test_create_and_add ---\n");

    ActivityList *list = createActivityList(3);
    check(list != NULL,       "createActivityList(3) не NULL");
    check(list->size == 0,    "size == 0 после создания");
    check(list->capacity == 3,"capacity == 3");

    check(addActivity(list, 1, 4, "A") == 1, "addActivity A");
    check(addActivity(list, 3, 5, "B") == 1, "addActivity B");
    check(addActivity(list, 0, 6, "C") == 1, "addActivity C");

    check(list->size == 3, "size == 3 после трёх добавлений");
    check(addActivity(list, 5, 7, "D") == 0,
          "addActivity за пределы capacity возвращает 0");

    check(list->items[0].start == 1, "A.start == 1");
    check(list->items[0].end   == 4, "A.end   == 4");
    check(strcmp(list->items[1].name, "B") == 0, "имя B хранится корректно");

    check(addActivity(NULL, 1, 2, "X") == 0, "addActivity(NULL) == 0");

    freeActivityList(list);
    printf("  OK  freeActivityList без крашей\n");
}

/* ============================================================ */
/*  Сортировка по времени окончания                             */
/* ============================================================ */
static void test_sort(void)
{
    printf("\n--- test_sort ---\n");

    ActivityList *list = createActivityList(5);
    addActivity(list, 0, 6, "C");
    addActivity(list, 1, 4, "A");
    addActivity(list, 3, 5, "B");
    addActivity(list, 5, 7, "D");
    addActivity(list, 3, 9, "E");

    sortByEndTime(list);

    int ok = 1;
    for (int i = 1; i < list->size; i++)
        if (list->items[i - 1].end > list->items[i].end)
            ok = 0;
    check(ok, "end не убывает после сортировки");

    check(strcmp(list->items[0].name, "A") == 0, "первый элемент — A [1,4]");
    check(strcmp(list->items[4].name, "E") == 0, "последний элемент — E [3,9]");

    sortByEndTime(list);  // повторная сортировка не ломает данные
    check(list->size == 5, "размер не изменился после повторной сортировки");

    freeActivityList(list);
}

/* ============================================================ */
/*  Жадный выбор — основной пример (A-K)                       */
/* ============================================================ */
static void test_select_basic(void)
{
    printf("\n--- test_select_basic ---\n");

    ActivityList *all = createActivityList(7);
    addActivity(all, 1,  4,  "A");
    addActivity(all, 3,  5,  "B");
    addActivity(all, 0,  6,  "C");
    addActivity(all, 5,  7,  "D");
    addActivity(all, 3,  9,  "E");
    addActivity(all, 8,  11, "H");
    addActivity(all, 12, 16, "K");

    ActivityList *sel = selectActivities(all);

    check(sel != NULL,    "selectActivities не NULL");
    check(sel->size == 4, "выбрано 4 активности (A,D,H,K)");
    check(strcmp(sel->items[0].name, "A") == 0, "1-я выбранная — A");
    check(strcmp(sel->items[1].name, "D") == 0, "2-я выбранная — D");
    check(strcmp(sel->items[2].name, "H") == 0, "3-я выбранная — H");
    check(strcmp(sel->items[3].name, "K") == 0, "4-я выбранная — K");

    // исходный список не мутируется
    check(all->size == 7, "исходный all->size == 7");
    check(strcmp(all->items[2].name, "C") == 0,
          "исходный all не изменён (C на своём месте)");

    freeActivityList(sel);
    freeActivityList(all);
}

/* ============================================================ */
/*  Конференц-зал — пример из lab2/README.md (единица = 30 мин) */
/* ============================================================ */
static void test_select_conference(void)
{
    printf("\n--- test_select_conference ---\n");

    ActivityList *all = createActivityList(6);
    addActivity(all, 18, 21, "Лекция");       // 09:00–10:30
    addActivity(all, 19, 22, "Митинг");       // 09:30–11:00
    addActivity(all, 21, 23, "Код-ревью");    // 10:30–11:30
    addActivity(all, 22, 24, "Обед");         // 11:00–12:00
    addActivity(all, 23, 26, "Презентация");  // 11:30–13:00
    addActivity(all, 24, 25, "Ретроспектива");// 12:00–12:30

    ActivityList *sel = selectActivities(all);

    check(sel->size == 3, "выбрано 3 из 6 мероприятий");
    check(strcmp(sel->items[0].name, "Лекция") == 0,      "1: Лекция");
    check(strcmp(sel->items[1].name, "Код-ревью") == 0,   "2: Код-ревью");
    check(strcmp(sel->items[2].name, "Ретроспектива") == 0,"3: Ретроспектива");

    freeActivityList(sel);
    freeActivityList(all);
}

/* ============================================================ */
/*  Крайние случаи                                              */
/* ============================================================ */
static void test_edge_cases(void)
{
    printf("\n--- test_edge_cases ---\n");

    /* Пустой список */
    ActivityList *empty = createActivityList(3);
    ActivityList *sel = selectActivities(empty);
    check(sel != NULL,   "выбор из пустого списка — не NULL");
    check(sel->size == 0,"из пустого списка выбрано 0");
    freeActivityList(sel);
    freeActivityList(empty);

    /* Одна активность */
    ActivityList *one = createActivityList(1);
    addActivity(one, 2, 8, "Solo");
    sel = selectActivities(one);
    check(sel->size == 1, "из одного элемента выбран 1");
    check(strcmp(sel->items[0].name, "Solo") == 0, "выбрана та самая");
    freeActivityList(sel);
    freeActivityList(one);

    /* Активности впритык: [0,5] и [5,10] не конфликтуют */
    ActivityList *touch = createActivityList(2);
    addActivity(touch, 0,  5, "X");
    addActivity(touch, 5, 10, "Y");
    sel = selectActivities(touch);
    check(sel->size == 2, "впритык [0,5] + [5,10] обе подходят");
    freeActivityList(sel);
    freeActivityList(touch);

    /* Одинаковое время окончания, одинаковое начало */
    ActivityList *same = createActivityList(3);
    addActivity(same, 5, 7, "P");
    addActivity(same, 5, 7, "Q");
    addActivity(same, 5, 7, "R");
    sel = selectActivities(same);
    check(sel->size == 1,
          "одинаковые интервалы — берётся только один");
    freeActivityList(sel);
    freeActivityList(same);

    /* Полное перекрытие */
    ActivityList *nested = createActivityList(3);
    addActivity(nested, 0, 10, "big");
    addActivity(nested, 1,  3, "small1");
    addActivity(nested, 4,  6, "small2");
    sel = selectActivities(nested);
    check(sel->size == 2,
          "полное перекрытие: small1 + small2 вместо big");
    check(strcmp(sel->items[0].name, "small1") == 0,
          "первая маленькая — small1");
    check(strcmp(sel->items[1].name, "small2") == 0,
          "вторая маленькая — small2");
    freeActivityList(sel);
    freeActivityList(nested);
}

/* ============================================================ */
/*  Выбор из неотсортированного списка                          */
/* ============================================================ */
static void test_unsorted_input(void)
{
    printf("\n--- test_unsorted_input ---\n");

    ActivityList *all = createActivityList(4);
    addActivity(all, 10, 12, "X");
    addActivity(all,  1,  3, "A");
    addActivity(all,  3,  5, "B");
    addActivity(all,  5,  7, "C");

    ActivityList *sel = selectActivities(all);

    check(sel->size == 4,   "4 непересекающихся из 4");
    check(strcmp(sel->items[0].name, "A") == 0, "1-я — A [1,3]");
    check(strcmp(sel->items[1].name, "B") == 0, "2-я — B [3,5]");
    check(strcmp(sel->items[2].name, "C") == 0, "3-я — C [5,7]");
    check(strcmp(sel->items[3].name, "X") == 0, "4-я — X [10,12]");

    // исходный порядок сохранился
    check(strcmp(all->items[0].name, "X") == 0,
          "исходный all не мутировал (X на месте)");

    freeActivityList(sel);
    freeActivityList(all);
}

/* ============================================================ */
/*  Визуализация (--demo)                                       */
/* ============================================================ */
static void run_demo(void)
{
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Демонстрация — Activity Selection (жадный алгоритм)   ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* Пример 1: Activity README */
    printf("\n▸ Пример 1 (из activity/README.md):\n\n");

    ActivityList *all = createActivityList(7);
    addActivity(all,  1,  4, "A");
    addActivity(all,  3,  5, "B");
    addActivity(all,  0,  6, "C");
    addActivity(all,  5,  7, "D");
    addActivity(all,  3,  9, "E");
    addActivity(all,  8, 11, "H");
    addActivity(all, 12, 16, "K");

    ActivityList *sel = selectActivities(all);

    printf("  Все активности:             Выбранные (%d шт.):\n", sel->size);
    for (int i = 0; i < all->size; i++)
    {
        int in_sel = 0;
        for (int j = 0; j < sel->size; j++)
            if (all->items[i].start == sel->items[j].start &&
                all->items[i].end   == sel->items[j].end &&
                strcmp(all->items[i].name, sel->items[j].name) == 0)
                in_sel = 1;

        if (in_sel)
            printf("  [*] %-3s: [%d, %d]\n",
                   all->items[i].name, all->items[i].start, all->items[i].end);
        else
            printf("      %-3s: [%d, %d]\n",
                   all->items[i].name, all->items[i].start, all->items[i].end);
    }

    printf("\n  Выбранные активности:\n");
    printActivityList(sel);

    printf("\n");
    printTimeline(all, sel);

    freeActivityList(sel);
    freeActivityList(all);

    /* Пример 2: Конференц-зал */
    printf("\n▸ Пример 2 (конференц-зал, единица = 30 мин):\n\n");

    ActivityList *conf = createActivityList(6);
    addActivity(conf, 18, 21, "Лекция");
    addActivity(conf, 19, 22, "Митинг");
    addActivity(conf, 21, 23, "Код-ревью");
    addActivity(conf, 22, 24, "Обед");
    addActivity(conf, 23, 26, "Презентация");
    addActivity(conf, 24, 25, "Ретроспектива");

    sel = selectActivities(conf);
    printf("  Выбрано: %d / 6 мероприятий\n", sel->size);
    printTimeline(conf, sel);

    freeActivityList(sel);
    freeActivityList(conf);
}

/* ============================================================ */
/*  main                                                        */
/* ============================================================ */
int main(int argc, char *argv[])
{
    printf("=== Activity Selection — тесты ===\n");

    test_create_and_add();
    test_sort();
    test_select_basic();
    test_select_conference();
    test_edge_cases();
    test_unsorted_input();

    printf("\n--- Итого: пройдено %d, провалено %d ---\n",
           tests_passed, tests_failed);

    if (argc > 1 && strcmp(argv[1], "--demo") == 0)
        run_demo();

    return tests_failed == 0 ? 0 : 1;
}