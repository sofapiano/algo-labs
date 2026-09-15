#include "lcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int maxInt(int a, int b)
{
    return (a > b) ? a : b;
}

static int **allocateTable(int rows, int cols)
{
}

static void freeTable(int **table, int rows)
{
}


int lcsLength(const char *s1, const char *s2)
{
}

LCSResult *longestCommonSubsequence(const char *s1, const char *s2)
{
}

char *reconstructLCS(int **dp, const char *s1, const char *s2, int i, int j)
{
}


void printLCSTable(LCSResult *result, const char *s1, const char *s2)
{
}

void highlightLCS(const char *s1, const char *s2, const char *lcs)
{
}


static int **computeLCSTableForLines(char **lines1, int count1, char **lines2, int count2)
{
}

DiffResult *diffLines(char **lines1, int count1, char **lines2, int count2)
{
}

// Вспомогательная функция: чтение файла в массив строк
static char **readFileLines(const char *filename, int *count)
{
}

static void freeFileLines(char **lines, int count)
{
}

DiffResult *diffFiles(const char *file1, const char *file2)
{
}

void printDiff(DiffResult *result)
{
}

// Освобождение памяти

void freeLCSResult(LCSResult *result)
{
}

void freeDiffResult(DiffResult *result)
{
}
