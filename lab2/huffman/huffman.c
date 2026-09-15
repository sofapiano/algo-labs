#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_CHARS 256


MinHeap *createMinHeap(int capacity)
{
}

static void swapNodes(HuffmanNode **a, HuffmanNode **b)
{
    // ну шо как поменять местами два значения переменных без третьей??))
    // а? а?
    // а без гпт догадаетесь?
}

static void heapifyUp(MinHeap *heap, int idx)
{
}

static void heapifyDown(MinHeap *heap, int idx)
{
}

void insertMinHeap(MinHeap *heap, HuffmanNode *node)
{
}

HuffmanNode *extractMin(MinHeap *heap)
{
}

void freeMinHeap(MinHeap *heap)
{
}

static HuffmanNode *createNode(char symbol, int frequency)
{
}

static int isLeaf(HuffmanNode *node)
{
    return node && !node->left && !node->right; // так и быть, оставил эту сложную функцию уже решённой
}


int *countFrequencies(const char *text, int *uniqueCount)
{
}

HuffmanNode *buildHuffmanTree(const char *text)
{
}

static void generateCodesRecursive(HuffmanNode *node, char *code, int depth, HuffmanCode *codes, int *count)
{
}

HuffmanTable *generateCodes(HuffmanNode *root)
{
}


static HuffmanCode *findCode(HuffmanTable *table, char symbol)
{
}

char *encode(const char *text, HuffmanTable *table)
{
}

char *decode(const char *encoded, HuffmanNode *root)
{
}

static void printTreeRecursive(HuffmanNode *node, char *prefix, int isLeft)
{
}

void printHuffmanTree(HuffmanNode *root)
{
}

void printHuffmanTable(HuffmanTable *table)
{
}

CompressionStats getCompressionStats(const char *text, HuffmanTable *table)
{
}

void printCompressionStats(CompressionStats *stats)
{
}

void freeHuffmanTree(HuffmanNode *root)
{
}

void freeHuffmanTable(HuffmanTable *table)
{
}
