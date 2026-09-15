#include "activity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ActivityList *createActivityList(int capacity)
{
    if (capacity < 0)
        return NULL;
    if (capacity == 0)
        capacity = 1;

    ActivityList *list = malloc(sizeof(ActivityList));
    if (list == NULL)
        return NULL;

    list->items = malloc(sizeof(Activity) * (size_t)capacity);
    if (list->items == NULL)
    {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = capacity;
    return list;
}

int addActivity(ActivityList *list, int start, int end, const char *name)
{
    if (list == NULL || name == NULL || list->size >= list->capacity)
        return 0;

    Activity *activity = &list->items[list->size];
    activity->start = start;
    activity->end = end;
    strncpy(activity->name, name, sizeof(activity->name) - 1);
    activity->name[sizeof(activity->name) - 1] = '\0';

    list->size++;
    return 1;
}

void freeActivityList(ActivityList *list)
{
    if (list == NULL)
        return;

    free(list->items);
    free(list);
}

void printActivityList(ActivityList *list)
{
    if (list == NULL)
    {
        printf("(NULL)\n");
        return;
    }

    for (int i = 0; i < list->size; i++)
    {
        printf("  %s [%d, %d]\n",
               list->items[i].name, list->items[i].start, list->items[i].end);
    }
}

static int compareByEndTime(const void *a, const void *b)
{
    const Activity *x = (const Activity *)a;
    const Activity *y = (const Activity *)b;

    if (x->end != y->end)
        return x->end - y->end;

    return x->start - y->start;
}

void sortByEndTime(ActivityList *list)
{
    if (list == NULL || list->size <= 1)
        return;

    qsort(list->items, (size_t)list->size, sizeof(Activity), compareByEndTime);
}

ActivityList *selectActivities(ActivityList *activities)
{
    if (activities == NULL)
        return createActivityList(1);

    ActivityList *result = createActivityList(activities->size);
    if (result == NULL)
        return NULL;

    if (activities->size == 0)
        return result;

    // Копируем активности в буфер и сортируем его,
    // чтобы не менять порядок в исходном списке.
    Activity *sorted = malloc(sizeof(Activity) * (size_t)activities->size);
    if (sorted == NULL)
    {
        freeActivityList(result);
        return NULL;
    }

    memcpy(sorted, activities->items, sizeof(Activity) * (size_t)activities->size);
    qsort(sorted, (size_t)activities->size, sizeof(Activity), compareByEndTime);

    // Берём активность с самым ранним окончанием,
    // затем любую, не конфликтующую с последней выбранной.
    result->items[result->size++] = sorted[0];
    int lastEnd = sorted[0].end;

    for (int i = 1; i < activities->size; i++)
    {
        if (sorted[i].start >= lastEnd)
        {
            result->items[result->size++] = sorted[i];
            lastEnd = sorted[i].end;
        }
    }

    free(sorted);
    return result;
}

static int isSelected(const Activity *activity, const ActivityList *selected)
{
    if (selected == NULL)
        return 0;

    for (int i = 0; i < selected->size; i++)
    {
        if (activity->start == selected->items[i].start &&
            activity->end == selected->items[i].end &&
            strcmp(activity->name, selected->items[i].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void printTimeline(ActivityList *all, ActivityList *selected)
{
    if (all == NULL || all->size == 0)
    {
        printf("Нет активностей для отображения.\n");
        return;
    }

    int minStart = all->items[0].start;
    int maxEnd = all->items[0].end;
    int nameWidth = (int)strlen(all->items[0].name);

    for (int i = 1; i < all->size; i++)
    {
        if (all->items[i].start < minStart)
            minStart = all->items[i].start;
        if (all->items[i].end > maxEnd)
            maxEnd = all->items[i].end;

        int len = (int)strlen(all->items[i].name);
        if (len > nameWidth)
            nameWidth = len;
    }

    int span = maxEnd - minStart;
    int unit = 1;
    while (span / unit > 80)
        unit *= 2; // Масштабируем, если диапазон слишком широкий
    int width = (span + unit - 1) / unit;

    printf("Временная шкала (1 символ = %d ед., диапазон [%d, %d]):\n",
           unit, minStart, maxEnd);

    for (int i = 0; i < all->size; i++)
    {
        const Activity *activity = &all->items[i];
        int marker = isSelected(activity, selected);

        // Отметка выбранной активности
        printf("%s", marker ? "[*] " : "    ");
        printf("%-*s ", nameWidth, activity->name);

        int offset = (activity->start - minStart) / unit;
        int len = (activity->end - activity->start) / unit;
        if (len < 1)
            len = 1;

        printf("%*s", offset, "");
        for (int j = 0; j < len; j++)
            putchar(marker ? '#' : '=');

        printf("  [%d, %d]\n", activity->start, activity->end);
    }

    // Ось времени
    printf("%*s", 4 + nameWidth + 1, "");
    for (int k = 0; k <= width; k++)
        putchar(k % 5 == 0 ? '+' : '-');
    printf("\n");

    printf("%*s", 4 + nameWidth + 1, "");
    for (int k = 0; k <= width; k++)
    {
        if (k % 5 == 0)
            printf("%d", minStart + k * unit);
        else
            putchar(' ');
    }
    printf("\n");
}