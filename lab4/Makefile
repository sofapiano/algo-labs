CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

# Общие компоненты: list/vector/hash_table из lab3 (там реализован HashTable)
LIST = ../lab3/lab3/list/generic.c
VECTOR = ../lab3/lab3/vector/generic.c
HASH = ../lab3/lab4/hash_table/generic.c
GRAPH_C = ../lab3/graph.c

LIBS = $(LIST) $(VECTOR) $(HASH)

all: spanning_tree u_tests

# graph.c компилируется без линковки (-c) и без main, чтобы не конфликтовать
graph.o: $(GRAPH_C)
	$(CC) $(CFLAGS) -DTESTING -c $(GRAPH_C) -o graph.o

spanning_tree: spanning_tree.c disjoint_set.c graph.o $(LIBS)
	$(CC) spanning_tree.c disjoint_set.c graph.o $(LIBS) -o spanning_tree $(CFLAGS) $(LDFLAGS)

u_tests: test_disjoint_set.c disjoint_set.c $(LIBS)
	$(CC) test_disjoint_set.c disjoint_set.c $(LIBS) -o u_tests $(CFLAGS) $(LDFLAGS)

run_tests: u_tests
	./u_tests

clean:
	rm -f spanning_tree u_tests graph.o

.PHONY: all clean run_tests