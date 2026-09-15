#include <stdio.h>

// Оптимизированное решение
// Сложность: O(n) времени, O(1) памяти
//
// Идея: нам нужны только два последних значения, а не вся таблица

int climbStairs(int n)
{
    if (n <= 1)
        return 1;

    int prev2 = 1;  // dp[i-2]
    int prev1 = 1;  // dp[i-1]

    for (int i = 2; i <= n; i++)
    {
        int curr = prev1 + prev2;
        prev2 = prev1;
        prev1 = curr;
    }

    return prev1;
}

int main()
{
    printf("Оптимизированное решение (O(1) памяти):\n\n");

    for (int i = 1; i <= 10; i++)
    {
        printf("Ступенек: %2d, способов: %d\n", i, climbStairs(i));
    }

    printf("\n--- Демонстрация работы переменных ---\n\n");

    int n = 10;
    int prev2 = 1;
    int prev1 = 1;

    printf("Начало: prev2 = %d (dp[0]), prev1 = %d (dp[1])\n\n", prev2, prev1);

    for (int i = 2; i <= n; i++)
    {
        int curr = prev1 + prev2;
        printf("i = %d: curr = prev1 + prev2 = %d + %d = %d\n",
               i, prev1, prev2, curr);
        prev2 = prev1;
        prev1 = curr;
        printf("        prev2 = %d, prev1 = %d\n\n", prev2, prev1);
    }

    printf("Ответ: %d способов\n", prev1);

    return 0;
}
