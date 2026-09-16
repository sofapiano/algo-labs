#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "search.h"

#define MAX_QUERY_TERMS 32
#define TOP_K 10

/* ---------------- токенизация запроса (как в preprocess.py) ---------------- */

static int tokenizeQuery(const char* query, char* out[], int max_terms) {
    size_t len = strlen(query);
    char*  buf = (char*)malloc(len + 1);
    if (!buf) return 0;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)query[i];
        buf[i] = (isalnum(c) || c == '_') ? (char)tolower(c) : ' ';
    }
    buf[len] = '\0';

    int n = 0;
    char* p = buf;
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        char* start = p;
        while (*p && *p != ' ') p++;
        size_t tlen = (size_t)(p - start);
        if (tlen > 2 && n < max_terms) {
            char* tok = (char*)malloc(tlen + 1);
            memcpy(tok, start, tlen);
            tok[tlen] = '\0';
            out[n++] = tok;
        }
    }
    free(buf);
    return n;
}

/* ---------------- пересечение posting list'ов ---------------- */

static int postingContains(Vector* list, int doc_id) {
    long lo = 0, hi = (long)list->size - 1;
    while (lo <= hi) {
        long mid = (lo + hi) / 2;
        PostingEntry* e = (PostingEntry*)getVectorItem(list, (size_t)mid);
        if (e->doc_id == doc_id) return 1;
        if (e->doc_id < doc_id) lo = mid + 1;
        else hi = mid - 1;
    }
    return 0;
}

Vector* intersectPostings(Vector** lists, int n) {
    Vector* out = createPostingList();
    if (!out || n <= 0) return out;

    Vector* base = lists[0];
    for (int i = 1; i < n; i++)
        if (lists[i] && lists[i]->size < base->size) base = lists[i];

    for (size_t k = 0; k < base->size; k++) {
        PostingEntry* e = (PostingEntry*)getVectorItem(base, k);
        int in_all = 1;
        for (int i = 0; i < n; i++) {
            if (lists[i] == base) continue;
            if (!lists[i] || !postingContains(lists[i], e->doc_id)) {
                in_all = 0;
                break;
            }
        }
        if (!in_all) continue;
        /* в списках могут быть дубликаты doc_id (одинаковый терм из title и body) */
        if (out->size > 0 &&
            ((PostingEntry*)getVectorItem(out, out->size - 1))->doc_id == e->doc_id)
            continue;
        appendPosting(out, e->doc_id, e->title);
    }
    return out;
}

/* ---------------- поиск ---------------- */

SearchResults* search(Index* idx, const char* query) {
    SearchResults* sr = (SearchResults*)calloc(1, sizeof(SearchResults));
    if (!sr) return NULL;
    sr->results = createVector(sizeof(SearchResult));
    if (!idx || !query) return sr;

    char* terms[MAX_QUERY_TERMS];
    int   n = tokenizeQuery(query, terms, MAX_QUERY_TERMS);
    if (n == 0) return sr;

    clock_t t0 = clock();

    Vector* lists[MAX_QUERY_TERMS];
    int     missing = 0;
    for (int i = 0; i < n; i++) {
        lists[i] = lookupTerm(idx, terms[i]);
        free(terms[i]);
        if (!lists[i]) missing = 1;
    }

    if (missing || n == 0) {
        clock_t t1 = clock();
        sr->time_ms = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
        return sr;
    }

    Vector* inter = intersectPostings(lists, n);
    sr->total = inter ? (int)inter->size : 0;
    int score = n; /* AND-семантика: совпали все слова запроса */

    size_t shown = inter ? inter->size : 0;
    if (shown > TOP_K) shown = TOP_K;
    for (size_t i = 0; i < shown; i++) {
        PostingEntry* e = (PostingEntry*)getVectorItem(inter, i);
        SearchResult r;
        r.doc_id = e->doc_id;
        strncpy(r.title, e->title, MAX_TITLE_LEN - 1);
        r.title[MAX_TITLE_LEN - 1] = '\0';
        r.score = score;
        appendVectorItem(sr->results, &r);
    }
    vectorFree(inter);

    clock_t t1 = clock();
    sr->time_ms = (double)(t1 - t0) * 1000.0 / CLOCKS_PER_SEC;
    return sr;
}

/* ---------------- вывод ---------------- */

void printResultsText(const SearchResults* sr) {
    if (!sr) return;
    printf("Время: %.2f мс | Найдено: %d документов\n\n",
           sr->time_ms, sr->total);
    if (!sr->results) return;
    for (size_t i = 0; i < sr->results->size && i < TOP_K; i++) {
        SearchResult* r = (SearchResult*)getVectorItem(sr->results, i);
        printf("%2zu. [id=%d] %s\n", i + 1, r->doc_id, r->title);
    }
}

static void jsonEscape(FILE* f, const char* s) {
    fputc('"', f);
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
            case '"':  fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\n': fputs("\\n", f);  break;
            case '\r': fputs("\\r", f);  break;
            case '\t': fputs("\\t", f);  break;
            default:
                if (c < 0x20) fprintf(f, "\\u%04x", (unsigned)c);
                else fputc(c, f);
        }
    }
    fputc('"', f);
}

void printResultsJSON(const SearchResults* sr) {
    if (!sr) { printf("{\"total\":0,\"time_ms\":0,\"results\":[]}\n"); return; }
    printf("{\"total\":%d,\"time_ms\":%.2f,\"results\":[",
           sr->total, sr->time_ms);
    size_t shown = sr->results ? sr->results->size : 0;
    if (shown > TOP_K) shown = TOP_K;
    for (size_t i = 0; i < shown; i++) {
        SearchResult* r = (SearchResult*)getVectorItem(sr->results, i);
        if (i > 0) printf(",");
        printf("{\"doc_id\":%d,\"title\":", r->doc_id);
        jsonEscape(stdout, r->title);
        printf(",\"score\":%d}", r->score);
    }
    printf("]}\n");
}

void freeSearchResults(SearchResults* sr) {
    if (!sr) return;
    vectorFree(sr->results);
    free(sr);
}