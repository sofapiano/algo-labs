#ifndef GRAPH_H
#define GRAPH_H

#include "lab3/list/generic.h"
#include "lab3/vector/generic.h"
#include "lab4/hash_table/generic.h"

#include <stddef.h>

typedef long long NodeId;

typedef struct
{
    GenericList *adj; // список исходящих рёбер (GraphEdge)
    NodeId id;
    double lat;
    double lon;
} GraphNode;

typedef struct
{
    int to; // индекс узла-назначения в vectors->nodes
    double length;
} GraphEdge;

typedef struct
{
    Vector *nodes;    // GraphNode
    HashTable *index; // NodeId -> int (индекс в nodes)
} Graph;

Graph *readGraph(const char *dir_path);

int findNearestNode(Graph *g, double lat, double lon);

size_t findShortestPath(Graph *g, int start, int end, int **path);

void freeGraph(Graph *g);

int graphHashId(const void *key);
int graphCmpId(const void *a, const void *b);

#endif // GRAPH_H