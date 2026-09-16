#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "index.h"
#include "../btree/btree.h"

#define MAX_TOKEN_LEN 512
#define MAX_TOKENS_PER_DOC 4096

/*
 * Слой индекса.
 * На данный момент реализован только btree-бэкенд; avl/rb вернут NULL
 * в createIndex (они не реализованы).
 */

TreeType parseType(const char* s) {
    if (s == NULL) return TREE_AVL;
    if (strcmp(s, "avl") == 0)            return TREE_AVL;
    if (strcmp(s, "rb") == 0 ||
        strcmp(s, "rbtree") == 0 ||
        strcmp(s, "red-black") == 0)      return TREE_RB;
    if (strcmp(s, "btree") == 0)          return TREE_BTREE;
    fprintf(stderr, "Предупреждение: неизвестный тип '%s', используем avl\n", s);
    return TREE_AVL;
}

const char* typeName(TreeType type) {
    switch (type) {
        case TREE_AVL:   return "avl";
        case TREE_RB:    return "rb";
        case TREE_BTREE: return "btree";
    }
    return "unknown";
}

Index* createIndex(TreeType type) {
    if (type != TREE_BTREE) {
        fprintf(stderr,
                "Бэкенд '%s' пока не реализован; доступен только 'btree'.\n",
                typeName(type));
        return NULL;
    }
    Index* idx = (Index*)malloc(sizeof(Index));
    if (!idx) return NULL;
    idx->type = type;
    idx->tree = createBTree();
    if (!idx->tree) {
        free(idx);
        return NULL;
    }
    return idx;
}

void insertTerm(Index* idx, const char* term, int doc_id, const char* title) {
    if (!idx || !idx->tree) return;
    if (idx->type == TREE_BTREE)
        btreeInsert((BTree*)idx->tree, term, doc_id, title);
}

Vector* lookupTerm(const Index* idx, const char* term) {
    if (!idx || !idx->tree) return NULL;
    if (idx->type == TREE_BTREE)
        return btreeSearch((const BTree*)idx->tree, term);
    return NULL;
}

void indexDocument(Index* idx, int doc_id, const char* title,
                   const char** tokens, int n_tokens) {
    for (int i = 0; i < n_tokens; i++) {
        if (tokens[i])
            insertTerm(idx, tokens[i], doc_id, title);
    }
}

void traverseIndex(const Index* idx,
                   void (*visit)(const char* key, Vector* postings, void* ctx),
                   void* ctx) {
    if (!idx || !idx->tree) return;
    if (idx->type == TREE_BTREE)
        btreeTraverse((const BTree*)idx->tree, visit, ctx);
}

/* ---------------- JSON-парсинг строк docs.jsonl ---------------- */

static void skipSpace(const char** p) {
    while (**p && isspace((unsigned char)**p)) (*p)++;
}

/* Вытаскивает JSON-строку (с обработкой escape-последовательностей). */
static int parseJsonString(const char** p, char* out, size_t out_size) {
    const char* s = *p;
    if (*s != '"') return -1;
    s++;
    size_t o = 0;
    while (*s && *s != '"') {
        char c = *s;
        if (c == '\\') {
            s++;
            switch (*s) {
                case '"':  c = '"';  break;
                case '\\': c = '\\'; break;
                case '/':  c = '/';  break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                case 'n':  c = '\n'; break;
                case 't':  c = '\t'; break;
                case 'r':  c = '\r'; break;
                case 'u': {
                    s++;
                    unsigned code = 0;
                    for (int k = 0; k < 4; k++) {
                        if (!isxdigit((unsigned char)*s)) return -1;
                        code = code * 16 + (unsigned)(isdigit((unsigned char)*s)
                            ? *s - '0'
                            : (tolower((unsigned char)*s) - 'a' + 10));
                        s++;
                    }
                    if (code < 0x80) {
                        c = (char)code;
                    } else if (code < 0x800) {
                        if (o + 2 > out_size) return -1;
                        out[o++] = (char)(0xC0 | (code >> 6));
                        c = (char)(0x80 | (code & 0x3F));
                    } else {
                        if (o + 3 > out_size) return -1;
                        out[o++] = (char)(0xE0 | (code >> 12));
                        out[o++] = (char)(0x80 | ((code >> 6) & 0x3F));
                        c = (char)(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: return -1;
            }
        }
        if (o + 2 > out_size) return -1;
        out[o++] = c;
        s++;
    }
    if (*s != '"') return -1;
    *p = s + 1;
    out[o] = '\0';
    return (int)o;
}

/* Находит поле "key": "value" в строке JSONL. */
static int extractField(const char* line, const char* key,
                        char* out, size_t out_size) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(line, pattern);
    if (!p) return -1;
    p += strlen(pattern);
    skipSpace(&p);
    if (*p != ':') return -1;
    p++;
    skipSpace(&p);
    return parseJsonString(&p, out, out_size);
}

/* Вытаскивает массив токенов ["a","b",...]. */
static int extractTokens(const char* line, char* out[], int max_tokens) {
    const char* p = strstr(line, "\"tokens\"");
    if (!p) return 0;
    p += 8;
    skipSpace(&p);
    if (*p != ':') return 0;
    p++;
    skipSpace(&p);
    if (*p != '[') return 0;
    p++;

    int n = 0;
    for (;;) {
        skipSpace(&p);
        if (*p == ']' || *p == '\0') break;
        char buf[MAX_TOKEN_LEN];
        if (parseJsonString(&p, buf, sizeof(buf)) < 0) break;
        if (buf[0] && n < max_tokens)
            out[n++] = strdup(buf);
        skipSpace(&p);
        if (*p == ',') p++;
    }
    return n;
}

/* ---------------- чтение docs.jsonl ---------------- */

static char* readLine(FILE* f) {
    size_t cap = 1024, len = 0;
    char*  buf = (char*)malloc(cap);
    if (!buf) return NULL;
    int c;
    while ((c = fgetc(f)) != EOF) {
        if (c == '\n') break;
        if (len + 2 > cap) {
            cap *= 2;
            char* grown = (char*)realloc(buf, cap);
            if (!grown) { free(buf); return NULL; }
            buf = grown;
        }
        buf[len++] = (char)c;
    }
    if (c == EOF && len == 0) { free(buf); return NULL; }
    buf[len] = '\0';
    return buf;
}

/* ---------------- сохранение / загрузка ---------------- */

typedef struct {
    FILE* f;
    int   terms;
} SaveCtx;

static void saveVisit(const char* key, Vector* postings, void* ctx) {
    SaveCtx* c = (SaveCtx*)ctx;
    fprintf(c->f, "T %s\t%zu\n", key, postings ? postings->size : 0);
    c->terms++;
    if (!postings) return;
    for (size_t i = 0; i < postings->size; i++) {
        PostingEntry* e = (PostingEntry*)getVectorItem(postings, i);
        char title[MAX_TITLE_LEN];
        strncpy(title, e->title, MAX_TITLE_LEN - 1);
        title[MAX_TITLE_LEN - 1] = '\0';
        for (char* p = title; *p; p++)
            if (*p == '\t' || *p == '\n' || *p == '\r') *p = ' ';
        fprintf(c->f, "P %d\t%s\n", e->doc_id, title);
    }
}

void saveIndex(const Index* idx, const char* path) {
    if (!idx) return;
    FILE* f = fopen(path, "w");
    if (!f) { perror("saveIndex"); return; }
    SaveCtx c = { f, 0 };
    fprintf(f, "# inverted index, type=%s\n", typeName(idx->type));
    traverseIndex(idx, saveVisit, &c);
    fprintf(f, "# terms=%d\n", c.terms);
    fclose(f);
}

Index* loadIndex(const char* path, TreeType type) {
    Index* idx = createIndex(type);
    if (!idx) return NULL;

    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Ошибка: не удалось открыть индекс '%s'\n", path);
        freeIndex(idx);
        return NULL;
    }

    char line[1 << 16];
    char term[2048] = {0};
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        if (len == 0 || line[0] == '#') continue;

        if (line[0] == 'T' && line[1] == ' ') {
            sscanf(line + 2, "%2047s", term);
        } else if (line[0] == 'P' && line[1] == ' ') {
            char* p = line + 2;
            int   doc_id = (int)strtol(p, &p, 10);
            if (*p == '\t') p++;
            char title[MAX_TITLE_LEN];
            strncpy(title, p, MAX_TITLE_LEN - 1);
            title[MAX_TITLE_LEN - 1] = '\0';
            btreeInsert((BTree*)idx->tree, term, doc_id, title);
        }
    }
    fclose(f);
    return idx;
}

void freeIndex(Index* idx) {
    if (!idx) return;
    if (idx->tree) {
        if (idx->type == TREE_BTREE)
            freeBTree((BTree*)idx->tree);
        else
            free(idx->tree);
    }
    free(idx);
}

/* ---------------- команда index ---------------- */

void runIndex(TreeType type, const char* data_path, const char* idx_path) {
    Index* idx = createIndex(type);
    if (!idx) return;

    FILE* f = fopen(data_path, "r");
    if (!f) {
        fprintf(stderr, "Ошибка: не удалось открыть '%s'\n", data_path);
        freeIndex(idx);
        return;
    }

    clock_t t0 = clock();
    int     docs = 0;
    char*   line;
    while ((line = readLine(f)) != NULL) {
        char doc_id_str[64] = {0};
        char title[MAX_TITLE_LEN] = {0};
        char* tokens[MAX_TOKENS_PER_DOC];

        int ok = extractField(line, "doc_id", doc_id_str, sizeof(doc_id_str)) >= 0 &&
                 extractField(line, "title", title, sizeof(title)) >= 0;
        int n_tokens = ok ? extractTokens(line, tokens, MAX_TOKENS_PER_DOC) : 0;

        indexDocument(idx, ok ? (int)strtol(doc_id_str, NULL, 10) : 0, title,
                      (const char**)tokens, n_tokens);
        for (int i = 0; i < n_tokens; i++) free(tokens[i]);
        free(line);
        docs++;

        if (docs % 10000 == 0)
            fprintf(stderr, "Проиндексировано документов: %d\n", docs);
    }
    fclose(f);

    clock_t t1 = clock();
    double  sec = (double)(t1 - t0) / CLOCKS_PER_SEC;
    int terms = (type == TREE_BTREE) ? ((BTree*)idx->tree)->size : 0;
    fprintf(stderr, "Готово: %d документов, %d уникальных термов за %.2f сек\n",
            docs, terms, sec);

    saveIndex(idx, idx_path);
    fprintf(stderr, "Индекс сохранён: %s\n", idx_path);
    freeIndex(idx);
}