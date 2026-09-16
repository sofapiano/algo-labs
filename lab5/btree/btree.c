#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btree.h"

#define BTREE_MIN_KEYS (BTREE_T - 1)

/*
 * B-tree (порядок BTREE_T):
 *   - узел хранит от t-1 до 2t-1 ключей (у корня — от 1),
 *   - все листья на одной высоте,
 *   - вставка через предварительный сплит полных узлов при спуске.
 * Ключ — строка-терм, в паре с ним живёт его posting list (Vector).
 *
 * Классический B-tree не допускает дубликатов ключей, поэтому перед вставкой
 * ключ ищется в дереве: если уже существует — документ добавляется в его
 * posting list, иначе вставляется новый ключ.
 */

static char* dupKey(const char* key) {
    size_t len = strlen(key) + 1;
    char*  copy = (char*)malloc(len);
    if (copy) memcpy(copy, key, len);
    return copy;
}

static BTreeNode* createBTreeNode(int is_leaf) {
    BTreeNode* node = (BTreeNode*)calloc(1, sizeof(BTreeNode));
    if (node) node->is_leaf = is_leaf;
    return node;
}

/* Разбивает полного ребёнка parent->children[i] пополам.
   Средний ключ (и его posting list) поднимается в parent. */
static void btreeSplitChild(BTreeNode* parent, int i) {
    BTreeNode* full  = parent->children[i];
    BTreeNode* right = createBTreeNode(full->is_leaf);

    right->n = BTREE_MIN_KEYS;
    for (int j = 0; j < BTREE_MIN_KEYS; j++) {
        right->keys[j]     = full->keys[j + BTREE_T];
        right->postings[j] = full->postings[j + BTREE_T];
    }
    if (!full->is_leaf) {
        for (int j = 0; j < BTREE_T; j++)
            right->children[j] = full->children[j + BTREE_T];
    }

    full->n = BTREE_MIN_KEYS;

    for (int j = parent->n; j > i; j--) {
        parent->children[j + 1] = parent->children[j];
        parent->keys[j]         = parent->keys[j - 1];
        parent->postings[j]     = parent->postings[j - 1];
    }
    parent->children[i + 1] = right;
    parent->keys[i]         = full->keys[BTREE_T - 1];
    parent->postings[i]     = full->postings[BTREE_T - 1];
    parent->n++;
}

/* Вставка заведомо отсутствующего в дереве ключа.
   Возвращает 1 (новый ключ добавлен). */
static int btreeInsertNonFull(BTreeNode* node, const char* key,
                              int doc_id, const char* title) {
    int i = node->n - 1;

    if (node->is_leaf) {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            node->keys[i + 1]     = node->keys[i];
            node->postings[i + 1] = node->postings[i];
            i--;
        }
        node->keys[i + 1]     = dupKey(key);
        node->postings[i + 1] = createPostingList();
        appendPosting(node->postings[i + 1], doc_id, title);
        node->n++;
        return 1;
    }

    while (i >= 0 && strcmp(key, node->keys[i]) < 0)
        i--;

    BTreeNode* child = node->children[i + 1];
    if (child->n == BTREE_MAX_KEYS) {
        btreeSplitChild(node, i + 1);
        if (strcmp(key, node->keys[i + 1]) > 0)
            i++;
        child = node->children[i + 1];
    }
    return btreeInsertNonFull(child, key, doc_id, title);
}

BTree* createBTree(void) {
    BTree* tree = (BTree*)calloc(1, sizeof(BTree));
    if (tree) tree->root = createBTreeNode(1);
    return tree;
}

void btreeInsert(BTree* tree, const char* key, int doc_id, const char* title) {
    if (!tree || !tree->root || !key) return;

    Vector* existing = btreeSearch(tree, key);
    if (existing) {
        appendPosting(existing, doc_id, title);
        return;
    }

    BTreeNode* root = tree->root;
    if (root->n == BTREE_MAX_KEYS) {
        BTreeNode* new_root = createBTreeNode(0);
        new_root->children[0] = root;
        tree->root = new_root;
        btreeSplitChild(new_root, 0);
    }
    tree->size += btreeInsertNonFull(tree->root, key, doc_id, title);
}

Vector* btreeSearch(const BTree* tree, const char* key) {
    if (!tree || !key) return NULL;

    BTreeNode* node = tree->root;
    while (node) {
        int i = 0;
        while (i < node->n && strcmp(key, node->keys[i]) > 0)
            i++;
        if (i < node->n && strcmp(key, node->keys[i]) == 0)
            return node->postings[i];
        if (node->is_leaf)
            return NULL;
        node = node->children[i];
    }
    return NULL;
}

static void btreeFreeNode(BTreeNode* node) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        free(node->keys[i]);
        vectorFree(node->postings[i]);
    }
    if (!node->is_leaf) {
        for (int i = 0; i <= node->n; i++)
            btreeFreeNode(node->children[i]);
    }
    free(node);
}

void freeBTree(BTree* tree) {
    if (!tree) return;
    btreeFreeNode(tree->root);
    free(tree);
}

static void btreeTraverseNode(const BTreeNode* node,
                              void (*visit)(const char*, Vector*, void*),
                              void* ctx) {
    if (!node) return;

    for (int i = 0; i < node->n; i++) {
        if (!node->is_leaf)
            btreeTraverseNode(node->children[i], visit, ctx);
        visit(node->keys[i], node->postings[i], ctx);
    }
    if (!node->is_leaf)
        btreeTraverseNode(node->children[node->n], visit, ctx);
}

void btreeTraverse(const BTree* tree,
                   void (*visit)(const char* key, Vector* postings, void* ctx),
                   void* ctx) {
    if (!tree) return;
    btreeTraverseNode(tree->root, visit, ctx);
}