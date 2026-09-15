#include <stdio.h>
#include <stdlib.h>

// Решение табуляцией (снизу вверх)
// Сложность: O(n) времени, O(n) памяти

int climbStairs(int n)
{
    if (n <= 1)
        return 1;

    int *dp = malloc((n + 1) * sizeof(int));
    if (!dp)
        return -1;

    // База: один способ стоять на месте, один способ шагнуть на 1
    dp[0] = 1;
    dp[1] = 1;

    // Заполняем таблицу
    for (int i = 2; i <= n; i++)
    {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    int result = dp[n];
    free(dp);
    return result;
}

void printDPTable(int n)
{
    if (n <= 1)
    {
        printf("dp[0] = 1, dp[1] = 1\n");
        return;
    }

    int *dp = malloc((n + 1) * sizeof(int));
    if (!dp)
        return;

    dp[0] = 1;
    dp[1] = 1;

    for (int i = 2; i <= n; i++)
    {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    // Печатаем заголовок
    printf("i:      ");
    for (int i = 0; i <= n; i++)
        printf("%4d ", i);
    printf("\n");

    // Печатаем значения
    printf("dp[i]:  ");
    for (int i = 0; i <= n; i++)
        printf("%4d ", dp[i]);
    printf("\n");

    free(dp);
}

int main()
{
    printf("Решение табуляцией (O(n)):\n\n");

    printf("Таблица DP для N = 10:\n");
    printDPTable(10);

    printf("\n--- Пошаговое заполнение ---\n\n");

    int dp[11];
    dp[0] = 1;
    dp[1] = 1;

    printf("База:\n");
    printf("  dp[0] = 1  (один способ 'стоять на месте')\n");
    printf("  dp[1] = 1  (один способ: шаг на 1)\n\n");

    printf("Заполнение:\n");
    for (int i = 2; i <= 10; i++)
    {
        dp[i] = dp[i - 1] + dp[i - 2];
        printf("  dp[%d] = dp[%d] + dp[%d] = %d + %d = %d\n",
               i, i - 1, i - 2, dp[i - 1], dp[i - 2], dp[i]);
    }

    printf("\nОтвет: dp[10] = %d способов\n", dp[10]);

    return 0;
}
