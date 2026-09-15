#ifndef EXTRA_TASKS_H
#define EXTRA_TASKS_H

#include "vector/generic.h"

// Структура для хранения слова и его частоты
typedef struct
{
    char word[100];
    int count;
} WordFrequency;

// Функция для преобразования текста в Bag-of-Words вектор
Vector *textToBoW(const char *text);

// Функция для вывода Bag-of-Words вектора
void printBoW(Vector *vocabulary);

// Функция для сравнения слов (используется в векторе)
int wordEquals(const void *a, const void *b);

// Преобразование слова в нижний регистр, удаление пунктуации
void normalizeWord(char *word);

#endif
