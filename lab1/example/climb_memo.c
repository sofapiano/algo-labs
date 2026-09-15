#include <stdio.h>
#include <stdlib.h>

// Решение с мемоизацией (сверху вниз)
// Сложность: O(n) времени, O(n) памяти

int climbMemo(int n, int *cache)
{
    if (n <= 1)
        return 1;

    // Уже считали? Возвращаем из кэша
    if (cache[n] != -1)
        return cache[n];

    // Считаем и запоминаем
    cache[n] = climbMemo(n - 1, cache) + climbMemo(n - 2, cache);
    return cache[n];
}

int climbStairs(int n)
{
    int *cache = malloc((n + 1) * sizeof(int));
    if (!cache)
        return -1;

    // Заполняем кэш значением "ещё не считали"
    for (int i = 0; i <= n; i++)
        cache[i] = -1;

    int result = climbMemo(n, cache);
    free(cache);
    return result;
}

int main()
{
    printf("Решение с мемоизацией (O(n)):\n\n");

    for (int i = 1; i <= 10; i++)
    {
        printf("Ступенек: %2d, способов: %d\n", i, climbStairs(i));
    }

    printf("\n--- Теперь большие значения (мгновенно!) ---\n\n");

    for (int i = 20; i <= 45; i += 5)
    {
        printf("Ступенек: %2d, способов: %d\n", i, climbStairs(i));
    }

    return 0;
}
