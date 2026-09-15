// Алгоритм Краскала: минимальное остовное дерево на графе зданий города.
// Читает data/buildings.csv (id, адрес, lat, lon), строит рёбра
// по k ближайшим соседям и записывает MST в data/spanning_tree.csv.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lab3/graph.h"
#include "disjoint_set.h"

#define MAX_LINE 8192
#define DEFAULT_NEIGHBORS 3

// Рёбро для MST: концы — node ids, длина — расстояние между зданиями
typedef struct
{
    long from;
    long to;
    double length;
} Edge;

// ---------- helpers for a long-keyed HashTable (для DSU) ----------

static int spanningHashLong(const void *key)
{
    long long v = (long long)*(const long *)key;
    v ^= v >> 32;
    return (int)v;
}

static int spanningCmpLong(const void *a, const void *b)
{
    return *(const long *)a == *(const long *)b;
}

// ---------- чтение зданий ----------

static char *nextTabField(char **cursor)
{
    if (cursor == NULL || *cursor == NULL)
    {
        return NULL;
    }
    char *field = *cursor;
    char *tab = strchr(*cursor, '\t');
    if (tab != NULL)
    {
        *tab = '\0';
        *cursor = tab + 1;
    }
    else
    {
        *cursor = NULL;
    }
    return field;
}

// Разбирает строку формата "id[\taddress]?\tlat\tlon". Колонок 3 или 4.
static int parseBuildingLine(char *line, long long *id, double *lat, double *lon)
{
    char *cursor = line;
    char *id_str = nextTabField(&cursor);
    char *field1 = nextTabField(&cursor);
    char *field2 = nextTabField(&cursor);
    char *field3 = nextTabField(&cursor);
    if (id_str == NULL || field2 == NULL)
    {
        return 0;
    }
    // 4 колонки: id, address, lat, lon; 3 колонки: id, lat, lon
    if (field3 == NULL)
    {
        char *end = NULL;
        strtod(field1, &end);
        if (end == field1)
        {
            return 0; // первая колонка не число — значит формат другой
        }
        *id = atoll(id_str);
        *lat = atof(field1);
        *lon = atof(field2);
    }
    else
    {
        *id = atoll(id_str);
        *lat = atof(field2);
        *lon = atof(field3);
    }
    return 1;
}

// Добавляет узел в граф из строки "id[\taddress]?\tlat\tlon"
static void addBuildingFromLine(Graph *g, char *line)
{
    long long id;
    double lat, lon;
    if (!parseBuildingLine(line, &id, &lat, &lon))
    {
        return; // пустые/битые строки
    }
    if (getItemHashTable(g->index, &id, graphHashId, graphCmpId) != NULL)
    {
        return; // дубликаты id
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

// Чтение зданий из TSV-файла в граф (без рёбер)
Graph *read_buildings(char *path_nodes)
{
    FILE *f = fopen(path_nodes, "r");
    if (f == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть %s\n", path_nodes);
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

    char line[MAX_LINE];
    // Первая строка может быть заголовком ("id\taddress\tlat\tlon")
    int first_is_header = 1;
    if (fgets(line, sizeof(line), f) != NULL)
    {
        char *cursor = line;
        char *first = nextTabField(&cursor);
        char *end = NULL;
        if (first != NULL)
        {
            strtoll(first, &end, 10);
            first_is_header = (end == first);
        }
    }
    else
    {
        fclose(f);
        return g;
    }

    if (!first_is_header)
    {
        addBuildingFromLine(g, line);
    }

    while (fgets(line, sizeof(line), f) != NULL)
    {
        addBuildingFromLine(g, line);
    }

    fclose(f);
    return g;
}

// ---------- построение рёбер по k ближайшим соседям ----------

// Вставка в отсортированные (по возрастанию dist) массивы кандидатов
static void insertCandidate(size_t *idx, double *dist, size_t max_k, size_t *count,
                            size_t j, double d)
{
    if (*count < max_k)
    {
        size_t pos = (*count)++;
        while (pos > 0 && dist[pos - 1] > d)
        {
            dist[pos] = dist[pos - 1];
            idx[pos] = idx[pos - 1];
            pos--;
        }
        dist[pos] = d;
        idx[pos] = j;
        return;
    }
    if (d >= dist[max_k - 1] || max_k == 0)
    {
        return;
    }
    size_t pos = max_k - 1;
    while (pos > 0 && dist[pos - 1] > d)
    {
        dist[pos] = dist[pos - 1];
        idx[pos] = idx[pos - 1];
        pos--;
    }
    dist[pos] = d;
    idx[pos] = j;
}

// Для каждого узла создаёт max_edges рёбер к ближайшим соседям
void build_edges(Graph *graph, unsigned char max_edges)
{
    if (graph == NULL || max_edges == 0)
    {
        return;
    }
    size_t n = graph->nodes->size;
    if (n == 0)
    {
        return;
    }

    size_t *best_idx = (size_t *)malloc(max_edges * sizeof(size_t));
    double *best_sq = (double *)malloc(max_edges * sizeof(double));
    if (best_idx == NULL || best_sq == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось выделить память под соседей\n");
        free(best_idx);
        free(best_sq);
        return;
    }

    for (size_t i = 0; i < n; i++)
    {
        GraphNode *node_i = (GraphNode *)getVectorItem(graph->nodes, i);
        size_t count = 0;
        for (size_t j = 0; j < n; j++)
        {
            if (i == j)
            {
                continue;
            }
            GraphNode *node_j = (GraphNode *)getVectorItem(graph->nodes, j);
            double dlat = node_i->lat - node_j->lat;
            double dlon = node_i->lon - node_j->lon;
            insertCandidate(best_idx, best_sq, max_edges, &count, j, dlat * dlat + dlon * dlon);
        }
        for (size_t e = 0; e < count; e++)
        {
            GraphEdge edge;
            edge.to = (int)best_idx[e];
            edge.length = sqrt(best_sq[e]);
            appendItem(node_i->adj, &edge);
        }
    }

    free(best_idx);
    free(best_sq);
}

// ---------- алгоритм Краскала ----------

static int cmpEdge(const void *a, const void *b)
{
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    if (ea->length < eb->length)
    {
        return -1;
    }
    if (ea->length > eb->length)
    {
        return 1;
    }
    return 0;
}

// Возвращает вектор рёбер минимального остовного дерева
Vector *kruskal_mst(Graph *graph)
{
    if (graph == NULL)
    {
        return NULL;
    }
    size_t n = graph->nodes->size;

    Vector *all_edges = createVector(sizeof(Edge));
    if (all_edges == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < n; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(graph->nodes, i);
        long from = (long)node->id;
        Node *it;
        for (it = node->adj->head; it != NULL; it = it->next)
        {
            GraphEdge *ge = (GraphEdge *)it->data;
            GraphNode *to_node = (GraphNode *)getVectorItem(graph->nodes, (size_t)ge->to);
            Edge edge;
            edge.from = from;
            edge.to = (long)to_node->id;
            edge.length = ge->length;
            appendVectorItem(all_edges, &edge);
        }
    }

    // 1. Сортируем рёбра по возрастанию веса
    qsort(all_edges->data, all_edges->size, sizeof(Edge), cmpEdge);

    // 2. Каждый узел — отдельное множество в DSU поверх HashTable
    HashTable *node_table = createHashTable(sizeof(long), sizeof(int));
    if (node_table == NULL)
    {
        vectorFree(all_edges);
        return NULL;
    }
    for (size_t i = 0; i < n; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(graph->nodes, i);
        long id = (long)node->id;
        int dummy = (int)i;
        setItemHashTable(node_table, &id, &dummy, spanningHashLong, spanningCmpLong);
    }
    DisjointSet *dsu = dsu_create_from_hash_table(node_table);
    freeHashTable(node_table);
    if (dsu == NULL)
    {
        vectorFree(all_edges);
        return NULL;
    }

    // 3. Перебираем рёбра: разные компоненты -> берём в MST
    Vector *mst = createVector(sizeof(Edge));
    for (size_t i = 0; i < all_edges->size; i++)
    {
        if (mst->size == n - 1 || n == 0)
        {
            break;
        }
        Edge *edge = (Edge *)getVectorItem(all_edges, i);
        DSNode *a = find_set_from_key(dsu, edge->from);
        DSNode *b = find_set_from_key(dsu, edge->to);
        if (a == NULL || b == NULL)
        {
            continue;
        }
        if (find_set(a) != find_set(b))
        {
            union_sets(a, b);
            appendVectorItem(mst, edge);
        }
    }

    vectorFree(all_edges);
    freeDisjointSet(dsu);
    return mst;
}

// ---------- запись результата ----------

void write_edges_to_file(Vector *edges, char *path_out)
{
    if (edges == NULL)
    {
        return;
    }
    FILE *f = fopen(path_out, "w");
    if (f == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть %s\n", path_out);
        return;
    }
    fprintf(f, "node_id_from\tnode_id_to\tlength\n");
    for (size_t i = 0; i < edges->size; i++)
    {
        Edge *edge = (Edge *)getVectorItem(edges, i);
        fprintf(f, "%ld\t%ld\t%.6f\n", edge->from, edge->to, edge->length);
    }
    fclose(f);
}

// ---------- main ----------

static size_t totalEdges(const Graph *g)
{
    size_t total = 0;
    for (size_t i = 0; i < g->nodes->size; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, i);
        total += listLength(node->adj);
    }
    return total;
}

int main(int argc, char **argv)
{
    char default_in[] = "data/buildings.csv";
    char default_out[] = "data/spanning_tree.csv";
    char *path_input = default_in;
    char *path_output = default_out;
    if (argc > 1)
    {
        path_input = argv[1];
    }
    if (argc > 2)
    {
        path_output = argv[2];
    }

    printf("Чтение графа из файла %s\n", path_input);
    Graph *graph = read_buildings(path_input);
    if (graph == NULL)
    {
        return 1;
    }
    printf("Граф успешно прочитан с числом узлов: %u\n", (unsigned)graph->nodes->size);

    printf("Создание связей между домами\n");
    build_edges(graph, DEFAULT_NEIGHBORS);
    size_t edge_count = totalEdges(graph);
    printf("Рёбра успешно созданы. Всего %u рёбер\n", (unsigned)edge_count);

    printf("Запуск алгоритма Краскала...\n");
    printf("  Всего рёбер: %u\n", (unsigned)edge_count);
    Vector *mst = kruskal_mst(graph);
    if (mst == NULL)
    {
        freeGraph(graph);
        return 1;
    }
    printf("  Рёбра отсортированы\n");

    double total_length = 0.0;
    for (size_t i = 0; i < mst->size; i++)
    {
        Edge *edge = (Edge *)getVectorItem(mst, i);
        total_length += edge->length;
    }
    printf("  Добавлено рёбер в MST: %u\n", (unsigned)mst->size);
    printf("✓ MST построено. Рёбер в остове: %u из %u\n", (unsigned)mst->size, (unsigned)edge_count);
    printf("  Суммарная длина рёбер: %.6f (усл. ед. координат)\n", total_length);

    write_edges_to_file(mst, path_output);
    printf("Остовное дерево успешно сохранено\n");

    vectorFree(mst);
    freeGraph(graph);
    return 0;
}