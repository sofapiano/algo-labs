#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graph.h"
#include "lab3/comparators.h"
#include "lab3/list/generic.h"
#include "lab4/hash_table/generic.h"

#define INF_DIST (1e18)

int graphHashId(const void *key)
{
    long long v = *(const long long *)key;
    return (int)(v ^ (v >> 32));
}

int graphCmpId(const void *a, const void *b)
{
    return *(const long long *)a == *(const long long *)b;
}

static char *nextCsvField(char **cursor)
{
    if (cursor == NULL || *cursor == NULL)
    {
        return NULL;
    }
    char *field = *cursor;
    char *comma = strchr(*cursor, ',');
    if (comma != NULL)
    {
        *comma = '\0';
        *cursor = comma + 1;
    }
    else
    {
        *cursor = NULL;
    }
    return field;
}

static int parseNodeLine(char *line, NodeId *id, double *lat, double *lon)
{
    char *cursor = line;
    char *id_str = nextCsvField(&cursor);
    char *lat_str = nextCsvField(&cursor);
    char *lon_str = nextCsvField(&cursor);
    if (id_str == NULL || lat_str == NULL || lon_str == NULL)
    {
        return 0;
    }
    *id = atoll(id_str);
    *lat = atof(lat_str);
    *lon = atof(lon_str);
    return 1;
}

static int parseEdgeLine(char *line, NodeId *from, NodeId *to,
                         double *length, int *oneway)
{
    char *cursor = line;
    char *from_str = nextCsvField(&cursor);
    char *to_str = nextCsvField(&cursor);
    char *len_str = nextCsvField(&cursor);
    char *oneway_str = nextCsvField(&cursor);
    if (from_str == NULL || to_str == NULL || len_str == NULL || oneway_str == NULL)
    {
        return 0;
    }
    *from = atoll(from_str);
    *to = atoll(to_str);
    *length = atof(len_str);
    *oneway = (strcmp(oneway_str, "True") == 0 ||
               strcmp(oneway_str, "true") == 0 ||
               strcmp(oneway_str, "1") == 0);
    return 1;
}

Graph *readGraph(const char *dir_path)
{
    char path[1024];
    char line[2048];
    FILE *f;

    snprintf(path, sizeof(path), "%s/nodes.csv", dir_path);
    f = fopen(path, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть %s\n", path);
        return NULL;
    }
    if (fgets(line, sizeof(line), f) == NULL)
    {
        fclose(f);
        fprintf(stderr, "Ошибка: пустой nodes.csv\n");
        return NULL;
    }

    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (g == NULL)
    {
        fclose(f);
        fprintf(stderr, "Ошибка: не удалось выделить память под граф\n");
        return NULL;
    }
    g->nodes = createVector(sizeof(GraphNode));
    g->index = createHashTable(sizeof(NodeId), sizeof(int));
    if (g->nodes == NULL || g->index == NULL)
    {
        fclose(f);
        freeGraph(g);
        return NULL;
    }
    while (fgets(line, sizeof(line), f) != NULL)
    {
        NodeId id;
        double lat, lon;
        if (!parseNodeLine(line, &id, &lat, &lon))
        {
            continue;
        }
        if (getItemHashTable(g->index, &id, graphHashId, graphCmpId) != NULL)
        {
            continue;
        }
        GraphNode node;
        node.id = id;
        node.lat = lat;
        node.lon = lon;
        node.adj = createList(sizeof(GraphEdge));
        int idx = (int)g->nodes->size;
        appendVectorItem(g->nodes, &node);
        setItemHashTable(g->index, &id, &idx, graphHashId, graphCmpId);
    }
    fclose(f);

    snprintf(path, sizeof(path), "%s/edges.csv", dir_path);
    f = fopen(path, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть %s\n", path);
        return g;
    }
    if (fgets(line, sizeof(line), f) == NULL)
    {
        fclose(f);
        return g;
    }
    while (fgets(line, sizeof(line), f) != NULL)
    {
        NodeId from, to;
        double length;
        int oneway;
        if (!parseEdgeLine(line, &from, &to, &length, &oneway))
        {
            continue;
        }
        int *from_idx = (int *)getItemHashTable(g->index, &from, graphHashId, graphCmpId);
        int *to_idx = (int *)getItemHashTable(g->index, &to, graphHashId, graphCmpId);
        if (from_idx == NULL || to_idx == NULL)
        {
            continue;
        }
        GraphEdge edge;
        edge.to = *to_idx;
        edge.length = length;
        GraphNode *from_node = (GraphNode *)getVectorItem(g->nodes, (size_t)*from_idx);
        appendItem(from_node->adj, &edge);

        if (!oneway)
        {
            GraphEdge rev;
            rev.to = *from_idx;
            rev.length = length;
            GraphNode *to_node = (GraphNode *)getVectorItem(g->nodes, (size_t)*to_idx);
            appendItem(to_node->adj, &rev);
        }
    }
    fclose(f);

    return g;
}

int findNearestNode(Graph *g, double lat, double lon)
{
    double best_sq = INF_DIST;
    int best = -1;
    for (size_t i = 0; i < g->nodes->size; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, i);
        double dlat = node->lat - lat;
        double dlon = node->lon - lon;
        double sq = dlat * dlat + dlon * dlon;
        if (sq < best_sq)
        {
            best_sq = sq;
            best = (int)i;
        }
    }
    return best;
}

typedef struct
{
    double dist;
    int node;
} HeapItem;

typedef struct
{
    HeapItem *data;
    size_t size;
    size_t cap;
} MinHeap;

static void heapInit(MinHeap *h)
{
    h->data = NULL;
    h->size = 0;
    h->cap = 0;
}

static int heapLess(const HeapItem *a, const HeapItem *b)
{
    if (a->dist != b->dist)
    {
        return a->dist < b->dist;
    }
    return a->node < b->node;
}

static void heapPush(MinHeap *h, double dist, int node)
{
    if (h->size == h->cap)
    {
        size_t new_cap = h->cap ? h->cap * 2 : 16;
        HeapItem *new_data = (HeapItem *)realloc(h->data, new_cap * sizeof(HeapItem));
        if (new_data == NULL)
        {
            fprintf(stderr, "Ошибка: не удалось расширить кучу\n");
            exit(EXIT_FAILURE);
        }
        h->data = new_data;
        h->cap = new_cap;
    }
    HeapItem item = {dist, node};
    size_t i = h->size++;
    while (i > 0)
    {
        size_t parent = (i - 1) / 2;
        if (heapLess(&h->data[parent], &item))
        {
            break;
        }
        h->data[i] = h->data[parent];
        i = parent;
    }
    h->data[i] = item;
}

static HeapItem heapTop(MinHeap *h)
{
    return h->data[0];
}

static void heapPop(MinHeap *h)
{
    HeapItem last = h->data[--h->size];
    size_t i = 0;
    while (1)
    {
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;
        size_t smallest = i;
        if (left < h->size && heapLess(&h->data[left], &h->data[smallest]))
        {
            smallest = left;
        }
        if (right < h->size && heapLess(&h->data[right], &h->data[smallest]))
        {
            smallest = right;
        }
        if (smallest == i)
        {
            break;
        }
        h->data[i] = h->data[smallest];
        i = smallest;
    }
    h->data[i] = last;
}

size_t findShortestPath(Graph *g, int start, int end, int **path)
{
    *path = NULL;
    size_t n = g->nodes->size;
    if (n == 0 || start < 0 || end < 0 || start >= (int)n || end >= (int)n)
    {
        return 0;
    }

    double *dist = (double *)malloc(n * sizeof(double));
    int *prev = (int *)malloc(n * sizeof(int));
    if (dist == NULL || prev == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось выделить память для Дейкстры\n");
        free(dist);
        free(prev);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++)
    {
        dist[i] = INF_DIST;
        prev[i] = -1;
    }

    dist[start] = 0.0;
    MinHeap heap;
    heapInit(&heap);
    heapPush(&heap, 0.0, start);

    while (heap.size > 0)
    {
        HeapItem top = heapTop(&heap);
        heapPop(&heap);
        if (top.dist > dist[top.node])
        {
            continue;
        }
        if (top.node == end)
        {
            break;
        }
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, (size_t)top.node);
        Node *it;
        for (it = node->adj->head; it != NULL; it = it->next)
        {
            GraphEdge *edge = (GraphEdge *)it->data;
            if (dist[top.node] + edge->length < dist[edge->to])
            {
                dist[edge->to] = dist[top.node] + edge->length;
                prev[edge->to] = top.node;
                heapPush(&heap, dist[edge->to], edge->to);
            }
        }
    }

    size_t len = 0;
    if (dist[end] < INF_DIST)
    {
        for (int v = end; v != -1; v = prev[v])
        {
            len++;
        }
        *path = (int *)malloc(len * sizeof(int));
        if (*path == NULL)
        {
            fprintf(stderr, "Ошибка: не удалось выделить память под путь\n");
            free(dist);
            free(prev);
            free(heap.data);
            exit(EXIT_FAILURE);
        }
        size_t k = len;
        for (int v = end; v != -1; v = prev[v])
        {
            (*path)[--k] = v;
        }
    }

    free(dist);
    free(prev);
    free(heap.data);
    return len;
}

void freeGraph(Graph *g)
{
    if (g == NULL)
    {
        return;
    }
    if (g->nodes != NULL)
    {
        for (size_t i = 0; i < g->nodes->size; i++)
        {
            GraphNode *node = (GraphNode *)getVectorItem(g->nodes, i);
            freeList(node->adj);
        }
        vectorFree(g->nodes);
    }
    freeHashTable(g->index);
    free(g);
}

#ifndef TESTING
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Использование: %s <папка_с_данными> <входной_файл> <выходной_файл>\n", argv[0]);
        return 1;
    }

    Graph *g = readGraph(argv[1]);
    if (g == NULL)
    {
        return 1;
    }

    FILE *in = fopen(argv[2], "r");
    if (in == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть входной файл %s\n", argv[2]);
        freeGraph(g);
        return 1;
    }
    double lat1, lon1, lat2, lon2;
    if (fscanf(in, "%lf %lf", &lat1, &lon1) != 2 ||
        fscanf(in, "%lf %lf", &lat2, &lon2) != 2)
    {
        fprintf(stderr, "Ошибка: некорректный входной файл %s\n", argv[2]);
        fclose(in);
        freeGraph(g);
        return 1;
    }
    fclose(in);

    int start = findNearestNode(g, lat1, lon1);
    int end = findNearestNode(g, lat2, lon2);

    int *path = NULL;
    size_t len = findShortestPath(g, start, end, &path);

    FILE *out = fopen(argv[3], "wb");
    if (out == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть выходной файл %s\n", argv[3]);
        free(path);
        freeGraph(g);
        return 1;
    }
    for (size_t i = 0; i < len; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, (size_t)path[i]);
        fprintf(out, "%.6lf %.6lf\n", node->lat, node->lon);
    }
    fclose(out);

    free(path);
    freeGraph(g);
    return 0;
}
#endif