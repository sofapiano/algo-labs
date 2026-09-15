#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "vector/generic.h"
#include "extra_tasks.h"

int wordEquals(const void *a, const void *b)
{
    // Реализовать (это компаратор) - для доп задания;
}

void normalizeWord(char *word)
{
    // Реализовать (это компаратор) - для доп задания;
}

Vector *textToBoW(const char *text)
{
    // Реализовать задачу векторизации
}

void printBoW(Vector *vocabulary) // вспомогательная функция для вывода результата
{
    printf("Bag-of-Words:\n");
    for (size_t i = 0; i < vocabulary->size; i++)
    {
        WordFrequency *wf = getVectorItem(vocabulary, i);
        printf("%s -> %d\n", wf->word, wf->count);
    }
}
