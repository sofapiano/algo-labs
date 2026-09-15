#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void assertPathIds(Graph *g, const int *path, size_t len,
                          const long long *expected_ids, size_t expected_len)
{
    assert(len == expected_len);
    for (size_t i = 0; i < len; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, (size_t)path[i]);
        assert(node->id == expected_ids[i]);
    }
}

static Graph *buildGraph(void)
{
    Graph *g = (Graph *)malloc(sizeof(Graph));
    assert(g != NULL);
    g->nodes = createVector(sizeof(GraphNode));
    g->index = createHashTable(sizeof(NodeId), sizeof(int));
    assert(g->nodes != NULL && g->index != NULL);
    return g;
}

static void addNode(Graph *g, NodeId id, double lat, double lon)
{
    GraphNode node;
    node.id = id;
    node.lat = lat;
    node.lon = lon;
    node.adj = createList(sizeof(GraphEdge));
    int idx = (int)g->nodes->size;
    appendVectorItem(g->nodes, &node);
    setItemHashTable(g->index, &id, &idx, graphHashId, graphCmpId);
}

static void addEdge(Graph *g, NodeId from, NodeId to, double length)
{
    int *from_idx = (int *)getItemHashTable(g->index, &from, graphHashId, graphCmpId);
    int *to_idx = (int *)getItemHashTable(g->index, &to, graphHashId, graphCmpId);
    assert(from_idx != NULL && to_idx != NULL);
    GraphEdge e;
    e.to = *to_idx;
    e.length = length;
    GraphNode *from_node = (GraphNode *)getVectorItem(g->nodes, (size_t)*from_idx);
    appendItem(from_node->adj, &e);
}

static void freeMemGraph(Graph *g)
{
    for (size_t i = 0; i < g->nodes->size; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, i);
        freeList(node->adj);
    }
    vectorFree(g->nodes);
    freeHashTable(g->index);
    free(g);
}

static void testNearestNode(void)
{
    Graph *g = buildGraph();
    addNode(g, 1, 59.000000, 30.000000);
    addNode(g, 2, 59.001000, 30.000000);
    addNode(g, 3, 59.000000, 30.001000);

    assert(findNearestNode(g, 59.000001, 30.000001) == 0);
    assert(findNearestNode(g, 59.000900, 30.000001) == 1);
    assert(findNearestNode(g, 59.000001, 30.000900) == 2);
    assert(findNearestNode(g, 59.000001, 30.000700) == 2);

    freeMemGraph(g);
}

static void testEmptyGraph(void)
{
    Graph *g = buildGraph();
    int *path = NULL;
    assert(findNearestNode(g, 0.0, 0.0) == -1);
    assert(findShortestPath(g, 0, 0, &path) == 0);
    assert(path == NULL);
    freeMemGraph(g);
}

static void testLinear(void)
{
    Graph *g = readGraph("tests/01_linear");
    assert(g != NULL);
    assert(g->nodes->size == 5);

    int start = findNearestNode(g, 59.000001, 30.000001);
    int end = findNearestNode(g, 59.003999, 30.000001);
    assert(start == 0 && end == 4);

    int *path = NULL;
    size_t len = findShortestPath(g, start, end, &path);
    assert(len == 5);

    long long ids[] = {1, 2, 3, 4, 5};
    assertPathIds(g, path, len, ids, ARRAY_SIZE(ids));

    char buf[512];
    size_t pos = 0;
    for (size_t i = 0; i < len; i++)
    {
        GraphNode *node = (GraphNode *)getVectorItem(g->nodes, (size_t)path[i]);
        pos += (size_t)snprintf(buf + pos, sizeof(buf) - pos, "%.6lf %.6lf\n", node->lat, node->lon);
    }
    assert(strcmp(buf,
                  "59.000000 30.000000\n"
                  "59.001000 30.000000\n"
                  "59.002000 30.000000\n"
                  "59.003000 30.000000\n"
                  "59.004000 30.000000\n") == 0);

    free(path);
    freeGraph(g);
}

static void testShortestWins(void)
{
    Graph *g = readGraph("tests/02_shortest_wins");
    assert(g != NULL);
    assert(g->nodes->size == 4);

    int start = findNearestNode(g, 59.000001, 30.000001);
    int end = findNearestNode(g, 59.001999, 30.000499);
    assert(start == 0 && end == 3);

    int *path = NULL;
    size_t len = findShortestPath(g, start, end, &path);
    assert(len == 3);

    long long ids[] = {1, 2, 4};
    assertPathIds(g, path, len, ids, ARRAY_SIZE(ids));

    free(path);
    freeGraph(g);
}

static void testNoPath(void)
{
    Graph *g = readGraph("tests/03_no_path");
    assert(g != NULL);

    int start = findNearestNode(g, 59.000001, 30.000001);
    int end = findNearestNode(g, 59.010999, 30.009999);
    assert(start == 0);
    assert(end == 3);

    int *path = NULL;
    size_t len = findShortestPath(g, start, end, &path);
    assert(len == 0);
    assert(path == NULL);

    freeGraph(g);
}

static void testSelfLoop(void)
{
    Graph *g = readGraph("tests/04_self_loop");
    assert(g != NULL);

    int start = findNearestNode(g, 59.000001, 30.000001);
    int end = findNearestNode(g, 59.001999, 30.000001);
    int *path = NULL;
    size_t len = findShortestPath(g, start, end, &path);
    assert(len == 3);

    long long ids[] = {1, 2, 3};
    assertPathIds(g, path, len, ids, ARRAY_SIZE(ids));

    free(path);
    freeGraph(g);
}

static void testOneway(void)
{
    Graph *g = buildGraph();
    addNode(g, 10, 0.0, 0.0);
    addNode(g, 20, 0.0, 0.001);
    addNode(g, 30, 0.001, 0.0);
    addEdge(g, 10, 20, 100.0);

    int *path = NULL;
    long long ids[] = {10, 20};
    size_t len;

    len = findShortestPath(g, 0, 1, &path);
    assert(len == 2);
    assertPathIds(g, path, len, ids, ARRAY_SIZE(ids));
    free(path);

    len = findShortestPath(g, 1, 0, &path);
    assert(len == 0);
    assert(path == NULL);

    freeMemGraph(g);
}

static void testSameStartEnd(void)
{
    Graph *g = buildGraph();
    addNode(g, 7, 59.5, 30.5);
    addNode(g, 8, 59.6, 30.6);
    addEdge(g, 7, 8, 50.0);
    addEdge(g, 8, 7, 50.0);

    int *path = NULL;
    size_t len = findShortestPath(g, 0, 0, &path);
    assert(len == 1);
    assert(path != NULL && path[0] == 0);
    free(path);

    len = findShortestPath(g, 1, 0, &path);
    assert(len == 2);
    long long ids[] = {8, 7};
    assertPathIds(g, path, len, ids, ARRAY_SIZE(ids));
    free(path);

    freeMemGraph(g);
}

int main(void)
{
    printf("testNearestNode\n"); fflush(stdout); testNearestNode();
    printf("testEmptyGraph\n"); fflush(stdout); testEmptyGraph();
    printf("testLinear\n"); fflush(stdout); testLinear();
    printf("testShortestWins\n"); fflush(stdout); testShortestWins();
    printf("testNoPath\n"); fflush(stdout); testNoPath();
    printf("testSelfLoop\n"); fflush(stdout); testSelfLoop();
    printf("testOneway\n"); fflush(stdout); testOneway();
    printf("testSameStartEnd\n"); fflush(stdout); testSameStartEnd();

    printf("All unit tests passed\n");
    return 0;
}