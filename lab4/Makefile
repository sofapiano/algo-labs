CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

LIBS = ../lab3/list/generic.c \
       ../lab3/vector/generic.c \
       ../lab4/hash_table/generic.c \
	   ../lab12/graph.c

all: spanning_tree u_tests

spanning_tree: spanning_tree.c disjoint_set.c $(LIBS)
	$(CC) spanning_tree.c disjoint_set.c $(LIBS) -o spanning_tree $(CFLAGS) $(LDFLAGS)

spanning_tree.o: spanning_tree.c
	$(CC) $(CFLAGS) -DTESTING -c spanning_tree.c disjoint_set.c -o spanning_tree.o

u_tests: test_disjoint_set.c disjoint_set.c $(LIBS)
	$(CC) test_disjoint_set.c disjoint_set.c $(LIBS) -o u_tests $(CFLAGS) $(LDFLAGS)

clean:
	rm -f spanning_tree u_tests spanning_tree.o

.PHONY: all clean