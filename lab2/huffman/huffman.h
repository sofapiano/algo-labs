#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>

// Узел дерева Хаффмана
typedef struct HuffmanNode {
    char symbol;                    // '\0' для внутренних узлов
    int frequency;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

// Код символа
typedef struct {
    char symbol;
    char code[256];                 // Битовая строка: "0110"
    int codeLength;
} HuffmanCode;

// Таблица кодов
typedef struct {
    HuffmanCode *codes;
    int size;
    HuffmanNode *root;              // Корень дерева для декодирования
} HuffmanTable;

// Мин-куча для построения дерева
typedef struct {
    HuffmanNode **nodes;
    int size;
    int capacity;
} MinHeap;

// Статистика сжатия
typedef struct {
    int originalBits;
    int compressedBits;
    double compressionRatio;
    double entropy;
} CompressionStats;


MinHeap *createMinHeap(int capacity);
void insertMinHeap(MinHeap *heap, HuffmanNode *node);
HuffmanNode *extractMin(MinHeap *heap);
void freeMinHeap(MinHeap *heap);

int *countFrequencies(const char *text, int *uniqueCount);

HuffmanNode *buildHuffmanTree(const char *text);

HuffmanTable *generateCodes(HuffmanNode *root);

char *encode(const char *text, HuffmanTable *table);

char *decode(const char *encoded, HuffmanNode *root);

// Печать дерева (ну типа ASCII-art плиз сделайте угарно, уверен вам накинут баллов)
void printHuffmanTree(HuffmanNode *root);

void printHuffmanTable(HuffmanTable *table);

CompressionStats getCompressionStats(const char *text, HuffmanTable *table);
void printCompressionStats(CompressionStats *stats);

void freeHuffmanTree(HuffmanNode *root);
void freeHuffmanTable(HuffmanTable *table);

#endif // HUFFMAN_H
