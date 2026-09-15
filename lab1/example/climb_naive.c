#include <stdio.h>

// Наивное рекурсивное решение
// Сложность: O(2^n) - экспоненциальная, не использовать для n > 30
int climbStairs(int n)
{
    if (n <= 1)
        return 1;

    return climbStairs(n - 1) + climbStairs(n - 2);
}

int main()
{
    printf("Наивное рекурсивное решение (O(2^n)):\n\n");

    for (int i = 1; i <= 10; i++)
    {
        printf("Ступенек: %2d, способов: %d\n", i, climbStairs(i));
    }

    printf("\n");
    printf("Попробуйте раскомментировать строку ниже и запустить снова.\n");
    printf("Приготовьтесь ждать...\n");

    // printf("Ступенек: 45, способов: %d\n", climbStairs(45));

    return 0;
}
