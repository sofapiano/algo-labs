#ifndef LCS_H
#define LCS_H

#include <stddef.h>


typedef struct {
    int length;           // Длина LCS
    char *subsequence;    // Сама подпоследовательность
    int **dpTable;        // Таблица DP для визуализации
    int rows;             // Количество строк (len(s1) + 1)
    int cols;             // Количество столбцов (len(s2) + 1)
} LCSResult;


typedef struct {
    char type;      // '+' добавлена, '-' удалена, ' ' без изменений
    char *line;     // Содержимое строки
} DiffLine;

typedef struct {
    DiffLine *lines;
    int count;
} DiffResult;

int lcsLength(const char *s1, const char *s2);

LCSResult *longestCommonSubsequence(const char *s1, const char *s2);

// Восстановление подпоследовательности из таблицы DP
// Возвращает новую строку (нужно освободить)
char *reconstructLCS(int **dp, const char *s1, const char *s2, int i, int j);

// Печать таблицы DP
void printLCSTable(LCSResult *result, const char *s1, const char *s2);

// Подсветка LCS в обеих строках (символы LCS в квадратных скобках)
void highlightLCS(const char *s1, const char *s2, const char *lcs);

// Построить diff двух массивов строк
DiffResult *diffLines(char **lines1, int count1, char **lines2, int count2);

// Построить diff двух текстовых файлов
DiffResult *diffFiles(const char *file1, const char *file2);

// Печать результата diff
void printDiff(DiffResult *result);

void freeLCSResult(LCSResult *result);
void freeDiffResult(DiffResult *result);

#endif // LCS_H
