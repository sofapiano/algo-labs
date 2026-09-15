#define _POSIX_C_SOURCE 200809L
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
    int **table = (int **)malloc(rows * sizeof(int *));
    if (!table)
        return NULL;
    for (int i = 0; i < rows; i++)
    {
        table[i] = (int *)calloc(cols, sizeof(int));
        if (!table[i])
        {
            for (int j = 0; j < i; j++)
                free(table[j]);
            free(table);
            return NULL;
        }
    }
    return table;
}

static void freeTable(int **table, int rows)
{
    if (!table)
        return;
    for (int i = 0; i < rows; i++)
        free(table[i]);
    free(table);
}

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
    if (!s1 || !s2)
        return -1;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    /* Оптимизация по памяти: храним только две строки — O(n) */
    int *prev = (int *)malloc((n + 1) * sizeof(int));
    int *curr = (int *)malloc((n + 1) * sizeof(int));
    if (!prev || !curr)
    {
        free(prev);
        free(curr);
        return -1;
    }

    for (int j = 0; j <= n; j++)
        prev[j] = j;

    for (int i = 1; i <= m; i++)
    {
        curr[0] = i;
        for (int j = 1; j <= n; j++)
        {
            if (s1[i - 1] == s2[j - 1])
                curr[j] = prev[j - 1];
            else
                curr[j] = 1 + minOfThree(prev[j],      /* удаление */
                                          curr[j - 1],  /* вставка */
                                          prev[j - 1]); /* замена */
        }
        int *tmp = prev;
        prev = curr;
        curr = tmp;
    }

    int result = prev[n];
    free(prev);
    free(curr);
    return result;
}


EditResult *levenshteinWithOperations(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return NULL;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    int **dp = allocateTable(m + 1, n + 1);
    if (!dp)
        return NULL;

    /* База */
    for (int i = 0; i <= m; i++)
        dp[i][0] = i;
    for (int j = 0; j <= n; j++)
        dp[0][j] = j;

    /* Заполнение таблицы */
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            if (s1[i - 1] == s2[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = 1 + minOfThree(dp[i - 1][j],      /* удаление */
                                           dp[i][j - 1],      /* вставка */
                                           dp[i - 1][j - 1]); /* замена */
        }
    }

    /* Восстановление операций — идём от dp[m][n] к dp[0][0] */
    int maxOps = m + n;
    EditOperation *ops = (EditOperation *)malloc(maxOps * sizeof(EditOperation));
    if (!ops)
    {
        freeTable(dp, m + 1);
        return NULL;
    }

    int opCount = 0;
    int i = m, j = n;
    while (i > 0 || j > 0)
    {
        if (i > 0 && j > 0 && s1[i - 1] == s2[j - 1])
        {
            /* Символы совпали — ничего не делаем */
            i--;
            j--;
        }
        else if (i > 0 && j > 0 && dp[i][j] == dp[i - 1][j - 1] + 1)
        {
            /* Замена */
            ops[opCount].type = OP_REPLACE;
            ops[opCount].position = i - 1;
            ops[opCount].oldChar = s1[i - 1];
            ops[opCount].newChar = s2[j - 1];
            opCount++;
            i--;
            j--;
        }
        else if (i > 0 && dp[i][j] == dp[i - 1][j] + 1)
        {
            /* Удаление */
            ops[opCount].type = OP_DELETE;
            ops[opCount].position = i - 1;
            ops[opCount].oldChar = s1[i - 1];
            ops[opCount].newChar = '\0';
            opCount++;
            i--;
        }
        else
        {
            /* Вставка */
            ops[opCount].type = OP_INSERT;
            ops[opCount].position = i;
            ops[opCount].oldChar = '\0';
            ops[opCount].newChar = s2[j - 1];
            opCount++;
            j--;
        }
    }

    /* Разворачиваем массив операций (шли от конца к началу) */
    for (int k = 0; k < opCount / 2; k++)
    {
        EditOperation tmp = ops[k];
        ops[k] = ops[opCount - 1 - k];
        ops[opCount - 1 - k] = tmp;
    }

    EditResult *result = (EditResult *)malloc(sizeof(EditResult));
    if (!result)
    {
        free(ops);
        freeTable(dp, m + 1);
        return NULL;
    }
    result->distance = dp[m][n];
    result->operations = ops;
    result->operationCount = opCount;

    freeTable(dp, m + 1);
    return result;
}


void printEditTable(int **dp, const char *s1, const char *s2)
{
    if (!dp || !s1 || !s2)
        return;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    printf("      \"\"");
    for (int j = 0; j < n; j++)
        printf("  %c", s2[j]);
    printf("\n");

    for (int i = 0; i <= m; i++)
    {
        if (i == 0)
            printf("  \"\"");
        else
            printf("  %c ", s1[i - 1]);
        for (int j = 0; j <= n; j++)
        {
            printf("  %d", dp[i][j]);
        }
        printf("\n");
    }
}

void printOperations(EditResult *result, const char *s1, const char *s2)
{
    if (!result)
        return;

    (void)s1;
    (void)s2;

    printf("Операции (%d шт.):\n", result->operationCount);
    for (int i = 0; i < result->operationCount; i++)
    {
        EditOperation op = result->operations[i];
        printf("  %d. ", i + 1);
        switch (op.type)
        {
            case OP_DELETE:
                printf("DELETE '%c' из позиции %d", op.oldChar, op.position);
                break;
            case OP_INSERT:
                printf("INSERT '%c' в позицию %d", op.newChar, op.position);
                break;
            case OP_REPLACE:
                printf("REPLACE '%c' -> '%c' в позиции %d", op.oldChar, op.newChar, op.position);
                break;
            case OP_NONE:
                break;
        }
        printf("\n");
    }
}

void printTransformation(EditResult *result, const char *s1, const char *s2)
{
    if (!result || !s1 || !s2)
        return;

    int len = (int)strlen(s1);
    char *buf = (char *)malloc((len + 2) * sizeof(char));
    if (!buf)
        return;
    strcpy(buf, s1);

    printf("Пошаговое преобразование:\n\n");
    printf("  Начало:  \"%s\"\n", buf);

    /* Позиции операций считаются относительно ИСХОДНОЙ строки s1.
     * Но по мере применения INSERT/DELETE буфер сдвигается, поэтому
     * нужно накапливать смещение: INSERT сдвигает всё, что после
     * него, на +1, DELETE — на -1. REPLACE смещения не меняет. */
    int offset = 0;

    for (int i = 0; i < result->operationCount; i++)
    {
        EditOperation op = result->operations[i];
        int pos = op.position + offset;
        switch (op.type)
        {
            case OP_DELETE:
                memmove(&buf[pos], &buf[pos + 1],
                        strlen(&buf[pos + 1]) + 1);
                printf("  Шаг %d:   \"%s\"  (DELETE '%c' из %d)\n",
                       i + 1, buf, op.oldChar, op.position);
                offset--;
                break;
            case OP_INSERT:
                len = (int)strlen(buf);
                memmove(&buf[pos + 1], &buf[pos],
                        strlen(&buf[pos]) + 1);
                buf[pos] = op.newChar;
                printf("  Шаг %d:   \"%s\"  (INSERT '%c' в %d)\n",
                       i + 1, buf, op.newChar, op.position);
                offset++;
                break;
            case OP_REPLACE:
                buf[pos] = op.newChar;
                printf("  Шаг %d:   \"%s\"  (REPLACE '%c'->'%c' в %d)\n",
                       i + 1, buf, op.oldChar, op.newChar, op.position);
                break;
            case OP_NONE:
                break;
        }
    }
    printf("  Итог:    \"%s\"\n", s2);
    free(buf);
}


char **findSimilarWords(const char *word, char **dictionary, int dictSize,
                        int maxDistance, int *resultCount)
{
    if (!word || !dictionary || dictSize <= 0 || !resultCount)
        return NULL;

    /* Сначала считаем подходящих, потом собираем */
    int capacity = 16;
    char **found = (char **)malloc(capacity * sizeof(char *));
    if (!found)
        return NULL;

    *resultCount = 0;
    for (int i = 0; i < dictSize; i++)
    {
        int d = levenshteinDistance(word, dictionary[i]);
        if (d >= 0 && d <= maxDistance)
        {
            if (*resultCount >= capacity)
            {
                capacity *= 2;
                char **tmp = (char **)realloc(found, capacity * sizeof(char *));
                if (!tmp)
                {
                    freeSimilarWords(found, *resultCount);
                    *resultCount = 0;
                    return NULL;
                }
                found = tmp;
            }
            found[*resultCount] = strdup(dictionary[i]);
            if (!found[*resultCount])
            {
                freeSimilarWords(found, *resultCount);
                *resultCount = 0;
                return NULL;
            }
            (*resultCount)++;
        }
    }

    if (*resultCount == 0)
    {
        free(found);
        return NULL;
    }
    return found;
}

void freeSimilarWords(char **words, int count)
{
    if (!words)
        return;
    for (int i = 0; i < count; i++)
        free(words[i]);
    free(words);
}


void freeEditResult(EditResult *result)
{
    if (!result)
        return;
    free(result->operations);
    free(result);
}