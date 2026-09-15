#include <stdio.h>

// Жадный алгоритм размена монет
// Работает для стандартных денежных систем (1, 5, 10, 25)
int coinChangeGreedy(int amount, int *coins, int num_coins, int *result)
{
    int count = 0;

    // Предполагаем, что coins отсортированы по убыванию
    for (int i = 0; i < num_coins && amount > 0; i++)
    {
        while (amount >= coins[i])
        {
            result[count++] = coins[i];
            amount -= coins[i];
        }
    }

    return (amount == 0) ? count : -1;
}

void printChange(int amount, int *coins, int num_coins)
{
    int result[100];
    int count = coinChangeGreedy(amount, coins, num_coins, result);

    printf("Сдача %d копеек: ", amount);
    if (count == -1)
    {
        printf("невозможно\n");
        return;
    }

    printf("%d монет (", count);
    for (int i = 0; i < count; i++)
    {
        printf("%d", result[i]);
        if (i < count - 1) printf(" + ");
    }
    printf(")\n");
}

int main()
{
    printf("=== Жадный алгоритм размена монет ===\n\n");

    int coins[] = {25, 10, 5, 1};  // Стандартная система (USA)
    int num_coins = 4;

    printf("Система монет: {25, 10, 5, 1}\n\n");

    printChange(41, coins, num_coins);
    printChange(30, coins, num_coins);
    printChange(99, coins, num_coins);
    printChange(100, coins, num_coins);

    printf("\n--- Пошаговая демонстрация для 41 ---\n\n");

    int amount = 41;
    printf("Начальная сумма: %d\n\n", amount);

    for (int i = 0; i < num_coins && amount > 0; i++)
    {
        if (amount >= coins[i])
        {
            int taken = 0;
            while (amount >= coins[i])
            {
                amount -= coins[i];
                taken++;
            }
            printf("Берём %d × %d = %d. Осталось: %d\n",
                   taken, coins[i], taken * coins[i], amount);
        }
    }

    return 0;
}
