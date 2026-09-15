#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <stddef.h>

typedef struct {
    int start;
    int end;
    char name[64];
} Activity;

typedef struct {
    Activity *items;
    int size;
    int capacity;
} ActivityList;


ActivityList *createActivityList(int capacity);

int addActivity(ActivityList *list, int start, int end, const char *name);

void freeActivityList(ActivityList *list);

void printActivityList(ActivityList *list);

void sortByEndTime(ActivityList *list);

// Жадный выбор активностей
// Возвращает новый список с выбранными активностями
ActivityList *selectActivities(ActivityList *activities);

// Печать временной шкалы (текстовая диаграмма Ганта)
void printTimeline(ActivityList *all, ActivityList *selected);

#endif // ACTIVITY_H
