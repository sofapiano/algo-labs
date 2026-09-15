#include "levenshtein.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- простой тестовый фреймворк ---------- */

static int testsRun = 0;
static int testsPassed = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        testsRun++;                                                   \
        if (cond) {                                                   \
            testsPassed++;                                            \
            printf("  [OK]   %s\n", msg);                             \
        } else {                                                      \
            printf("  [FAIL] %s (line %d)\n", msg, __LINE__);         \
        }                                                             \
    } while (0)

/* Применяет операции из EditResult к копии s1 и возвращает
 * получившуюся строку. Используется, чтобы проверить не только
 * количество операций, но и то, что они реально превращают
 * s1 в s2. */
static char *applyOperations(EditResult *result, const char *s1)
{
    int cap = (int)strlen(s1) + result->operationCount + 1;
    char *buf = (char *)malloc(cap);
    strcpy(buf, s1);

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
                offset--;
                break;
            case OP_INSERT:
                memmove(&buf[pos + 1], &buf[pos],
                        strlen(&buf[pos]) + 1);
                buf[pos] = op.newChar;
                offset++;
                break;
            case OP_REPLACE:
                buf[pos] = op.newChar;
                break;
            case OP_NONE:
                break;
        }
    }
    return buf;
}

/* ---------- тесты levenshteinDistance ---------- */

static void testDistanceBasic(void)
{
    printf("\n-- levenshteinDistance: базовые случаи --\n");
    CHECK(levenshteinDistance("kitten", "sitting") == 3, "kitten -> sitting == 3");
    CHECK(levenshteinDistance("SATURDAY", "SUNDAY") == 3, "SATURDAY -> SUNDAY == 3");
    CHECK(levenshteinDistance("", "") == 0, "\"\" -> \"\" == 0");
    CHECK(levenshteinDistance("abc", "abc") == 0, "одинаковые строки == 0");
    CHECK(levenshteinDistance("", "abc") == 3, "\"\" -> \"abc\" == 3 (только вставки)");
    CHECK(levenshteinDistance("abc", "") == 3, "\"abc\" -> \"\" == 3 (только удаления)");
    CHECK(levenshteinDistance("a", "b") == 1, "\"a\" -> \"b\" == 1 (замена)");
}

static void testDistanceEdgeCases(void)
{
    printf("\n-- levenshteinDistance: граничные случаи --\n");
    CHECK(levenshteinDistance(NULL, "abc") == -1, "NULL s1 -> -1");
    CHECK(levenshteinDistance("abc", NULL) == -1, "NULL s2 -> -1");
    CHECK(levenshteinDistance(NULL, NULL) == -1, "NULL, NULL -> -1");
    CHECK(levenshteinDistance("aaaa", "aaaa") == 0, "повторяющиеся символы, равны");
    CHECK(levenshteinDistance("abcdef", "azced") >= 0, "произвольная пара не падает");
}

/* ---------- тесты levenshteinWithOperations ---------- */

static void checkTransformation(const char *s1, const char *s2, const char *label)
{
    EditResult *result = levenshteinWithOperations(s1, s2);
    CHECK(result != NULL, label);
    if (!result)
        return;

    int expectedDistance = levenshteinDistance(s1, s2);
    char msg[128];

    snprintf(msg, sizeof(msg), "%s: distance == operationCount == %d", label, expectedDistance);
    CHECK(result->distance == expectedDistance &&
          result->operationCount == expectedDistance, msg);

    char *transformed = applyOperations(result, s1);
    snprintf(msg, sizeof(msg), "%s: применение операций даёт s2", label);
    CHECK(strcmp(transformed, s2) == 0, msg);

    free(transformed);
    freeEditResult(result);
}

static void testOperations(void)
{
    printf("\n-- levenshteinWithOperations --\n");
    checkTransformation("kitten", "sitting", "kitten->sitting");
    checkTransformation("SATURDAY", "SUNDAY", "SATURDAY->SUNDAY");
    checkTransformation("", "abc", "пустая->abc (только INSERT)");
    checkTransformation("abc", "", "abc->пустая (только DELETE)");
    checkTransformation("abc", "abc", "равные строки (0 операций)");
    checkTransformation("flaw", "lawn", "flaw->lawn");

    testsRun++;
    if (levenshteinWithOperations(NULL, "abc") == NULL) {
        testsPassed++;
        printf("  [OK]   NULL s1 -> NULL\n");
    } else {
        printf("  [FAIL] NULL s1 -> NULL (line %d)\n", __LINE__);
    }
}

/* ---------- тесты findSimilarWords ---------- */

static void testFindSimilarWords(void)
{
    printf("\n-- findSimilarWords --\n");
    char *dict[] = {"algorithm", "programming", "computer", "function"};
    int dictSize = 4;
    int count = 0;

    char **found = findSimilarWords("algortihm", dict, dictSize, 2, &count);
    CHECK(found != NULL && count == 1, "опечатка 'algortihm' находит 1 совпадение");
    if (found)
    {
        CHECK(strcmp(found[0], "algorithm") == 0, "найденное слово == 'algorithm'");
        freeSimilarWords(found, count);
    }

    count = 0;
    char **none = findSimilarWords("zzzzzzzzzz", dict, dictSize, 1, &count);
    CHECK(none == NULL && count == 0, "нет совпадений при слишком строгом maxDistance");

    count = 0;
    char **all = findSimilarWords("qwe", dict, dictSize, 100, &count);
    CHECK(all != NULL && count == dictSize, "большой maxDistance находит все слова словаря");
    if (all)
        freeSimilarWords(all, count);
}

/* ---------- демонстрация ---------- */

static void runDemo(void)
{
    printf("=== SATURDAY -> SUNDAY ===\n\n");
    int dist = levenshteinDistance("SATURDAY", "SUNDAY");
    printf("Расстояние Левенштейна: %d\n\n", dist);

    EditResult *result = levenshteinWithOperations("SATURDAY", "SUNDAY");
    if (result)
    {
        printOperations(result, "SATURDAY", "SUNDAY");
        printf("\n");
        printTransformation(result, "SATURDAY", "SUNDAY");
        freeEditResult(result);
    }

    printf("\n=== Исправление опечаток ===\n\n");
    char *dict[] = {"algorithm", "programming", "computer", "function"};
    int count = 0;
    char **similar = findSimilarWords("algortihm", dict, 4, 2, &count);
    printf("Слово: \"algortihm\"\n");
    printf("Похожие слова в словаре (maxDistance=2): ");
    if (similar)
    {
        for (int i = 0; i < count; i++)
            printf("%s%s", similar[i], (i + 1 < count) ? ", " : "\n");
        freeSimilarWords(similar, count);
    }
    else
    {
        printf("нет\n");
    }
}

/* ---------- main ---------- */

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--demo") == 0)
    {
        runDemo();
        return 0;
    }

    testDistanceBasic();
    testDistanceEdgeCases();
    testOperations();
    testFindSimilarWords();

    printf("\n========================================\n");
    printf("Итого: %d/%d тестов пройдено\n", testsPassed, testsRun);
    printf("========================================\n");

    return (testsPassed == testsRun) ? 0 : 1;
}