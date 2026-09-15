# Задача 3: Наидлиннейшая общая подпоследовательность (LCS)

## Описание

**Подпоследовательность** — последовательность символов, полученная удалением некоторых (возможно, нуля) символов из строки без изменения порядка оставшихся.

Например, "ACE" — подпоследовательность "ABCDE", а "AEC" — нет.

**LCS (Longest Common Subsequence)** — наидлиннейшая подпоследовательность, общая для двух строк.

Применение: diff в системах контроля версий, сравнение ДНК, обнаружение плагиата.

## Рекуррентное соотношение

```
dp[i][j] = длина LCS для s1[0..i-1] и s2[0..j-1]

Если s1[i-1] == s2[j-1]:
    dp[i][j] = dp[i-1][j-1] + 1   // символ входит в LCS
Иначе:
    dp[i][j] = max(dp[i-1][j], dp[i][j-1])

База: dp[i][0] = dp[0][j] = 0
```

## Файлы

```
lcs/
├── lcs.h       # Заголовочный файл
├── lcs.c       # Реализация LCS + Diff
├── tests.c     # Тесты
└── README.md   # Этот файл
```

## Сборка и запуск

### Запуск тестов

```bash
gcc -Wall -Wextra -std=c11 -o tests lcs.c tests.c
./tests
```

### Демонстрация работы

```bash
./tests --demo
```

Выводит:
1. LCS для "AGGTAB" и "GXTXAYB" с таблицей DP и визуализацией
2. Diff двух фрагментов кода

## API

### Структуры LCS

```c
typedef struct {
    int length;           // Длина LCS
    char *subsequence;    // Сама подпоследовательность
    int **dpTable;        // Таблица DP
    int rows, cols;       // Размеры таблицы
} LCSResult;
```

### Структуры Diff

```c
typedef struct {
    char type;      // '+' добавлена, '-' удалена, ' ' без изменений
    char *line;     // Содержимое строки
} DiffLine;

typedef struct {
    DiffLine *lines;
    int count;
} DiffResult;
```

### Функции LCS

```c
// Только длина — O(m*n) времени, O(n) памяти
int lcsLength(const char *s1, const char *s2);

// Полный результат с таблицей — O(m*n) времени и памяти
LCSResult *longestCommonSubsequence(const char *s1, const char *s2);

// Восстановление подпоследовательности из таблицы
char *reconstructLCS(int **dp, const char *s1, const char *s2, int i, int j);
```

### Функции Diff

```c
// Diff двух массивов строк
DiffResult *diffLines(char **lines1, int count1, char **lines2, int count2);

// Diff двух файлов
DiffResult *diffFiles(const char *file1, const char *file2);

// Печать результата
void printDiff(DiffResult *result);
```

### Визуализация

```c
// Печать таблицы DP
void printLCSTable(LCSResult *result, const char *s1, const char *s2);

// Подсветка LCS в строках: [G]X[T]X[A]Y[B]
void highlightLCS(const char *s1, const char *s2, const char *lcs);
```

## Пример использования

```c
#include "lcs.h"
#include <stdio.h>

int main()
{
    const char *s1 = "AGGTAB";
    const char *s2 = "GXTXAYB";

    LCSResult *result = longestCommonSubsequence(s1, s2);

    printf("LCS: %s (длина %d)\n", result->subsequence, result->length);
    highlightLCS(s1, s2, result->subsequence);

    freeLCSResult(result);
    return 0;
}
```

## Пример вывода

```
=== LCS: AGGTAB vs GXTXAYB ===

Длина LCS: 4
LCS: "GTAB"

Таблица DP:
       ""  G  X  T  X  A  Y  B
 ""  0  0  0  0  0  0  0  0
  A  0  0  0  0  0  1  1  1
  G  0  1  1  1  1  1  1  1
  G  0  1  1  1  1  1  1  1
  T  0  1  1  2  2  2  2  2
  A  0  1  1  2  2  3  3  3
  B  0  1  1  2  2  3  3  4

Визуализация:
  s1: A[G]G[T][A][B]
  s2: [G]X[T]X[A]Y[B]
```

## Пример Diff

```
Diff:
  #include <stdio.h>
+ #include <stdlib.h>

  int main() {
-     printf("Hello");
+     printf("Hello, World!");
      return 0;
  }
```

## Сложность

| Функция | Время | Память |
|---------|-------|--------|
| `lcsLength` | O(m × n) | O(n) |
| `longestCommonSubsequence` | O(m × n) | O(m × n) |
| `diffLines` | O(k₁ × k₂ × L) | O(k₁ × k₂) |

где m, n — длины строк; k₁, k₂ — количество строк; L — средняя длина строки.

## Связь LCS и Diff

Алгоритм diff использует LCS для определения, какие строки остались без изменений. Строки из LCS помечаются как unchanged (` `), остальные — как added (`+`) или removed (`-`).
