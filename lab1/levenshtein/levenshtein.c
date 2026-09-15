#include "levenshtein.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int minInt(int a, int b)
{
    return (a < b) ? a : b;
}

static int minOfThree(int a, int b, int c)
{
    return minInt(minInt(a, b), c);
}

static int **allocateTable(int rows, int cols)
{
}

static void freeTable(int **table, int rows)
{
}

// Для отладки — возвращает имя операции
const char *operationName(OperationType type)
{
    switch (type)
    {
        case OP_NONE:    return "NONE";
        case OP_INSERT:  return "INSERT";
        case OP_DELETE:  return "DELETE";
        case OP_REPLACE: return "REPLACE";
        default:         return "UNKNOWN";
    }
}


int levenshteinDistance(const char *s1, const char *s2)
{
}

EditResult *levenshteinWithOperations(const char *s1, const char *s2)
{
}


void printEditTable(int **dp, const char *s1, const char *s2)
{
}

void printOperations(EditResult *result, const char *s1, const char *s2)
{
}

void printTransformation(EditResult *result, const char *s1, const char *s2)
{
}

char **findSimilarWords(const char *word, char **dictionary, int dictSize,
                        int maxDistance, int *resultCount)
{
}

void freeSimilarWords(char **words, int count)
{
}


void freeEditResult(EditResult *result)
{
}
