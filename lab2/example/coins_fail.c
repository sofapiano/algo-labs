#include <stdio.h>
#include <stdlib.h>

// Жадный алгоритм — не всегда оптимален!
int coinChangeGreedy(int amount, int *coins, int n, int *result)
{
    int count = 0;
    for (int i = 0; i < n && amount > 0; i++)
    {
        while (amount >= coins[i])
        {
            result[count++] = coins[i];
            amount -= coins[i];
        }
    }
    return (amount == 0) ? count : -1;
}

// Оптимальное решение через DP
int coinChangeDP(int amount, int *coins, int n, int *result)
{
    int *dp = calloc(amount + 1, sizeof(int));
    int *parent = calloc(amount + 1, sizeof(int));

    for (int i = 1; i <= amount; i++)
    {
        dp[i] = amount + 1;
        parent[i] = -1;
    }

    dp[0] = 0;

    for (int i = 1; i <= amount; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (coins[j] <= i && dp[i - coins[j]] + 1 < dp[i])
            {
                dp[i] = dp[i - coins[j]] + 1;
                parent[i] = coins[j];
            }
        }
    }

    int count = 0;
    if (dp[amount] <= amount)
    {
        // Восстанавливаем решение
        int curr = amount;
        while (curr > 0 && parent[curr] != -1)
        {
            result[count++] = parent[curr];
            curr -= parent[curr];
        }
    }

    int res = (dp[amount] > amount) ? -1 : dp[amount];
    free(dp);
    free(parent);
    return res;
}

void printCoins(int *coins, int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%d", coins[i]);
        if (i < count - 1) printf(" + ");
    }
}

int main()
{
    printf("=== Когда жадный алгоритм НЕ работает ===\n\n");

    int coins[] = {4, 3, 1};  // Проблемная система
    int n = 3;

    printf("Система монет: {4, 3, 1}\n\n");

    int test_amounts[] = {6, 9, 11, 12};
    int num_tests = 4;

    for (int t = 0; t < num_tests; t++)
    {
        int amount = test_amounts[t];

        int greedy_result[100];
        int greedy_count = coinChangeGreedy(amount, coins, n, greedy_result);

        int dp_result[100];
        int dp_count = coinChangeDP(amount, coins, n, dp_result);

        printf("Сумма: %d\n", amount);
        printf("  Жадный:      %d монет (", greedy_count);
        printCoins(greedy_result, greedy_count);
        printf(")\n");

        printf("  Оптимальный: %d монет (", dp_count);
        printCoins(dp_result, dp_count);
        printf(")\n");

        if (greedy_count > dp_count)
            printf("    Жадный проиграл!\n");
        else
            printf("   Совпадает\n");

        printf("\n");
    }

    printf("--- Вывод ---\n");
    printf("Для системы {4, 3, 1} жадный алгоритм не всегда оптимален.\n");
    printf("Для сумм вида 3k (6, 9, 12, ...) лучше использовать только тройки.\n");

    return 0;
}
