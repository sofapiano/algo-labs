CC = gcc
CFLAGS = -lm

LIBS = lab3/list/generic.c \
	lab3/vector/generic.c \
	lab4/hash_table/generic.c

all: path_processor u_tests

path_processor: graph.c $(LIBS)
	$(CC) graph.c $(LIBS) -o path_processor $(CFLAGS)

# graph.c компилируется без линковки (-c), чтобы исключить его main при сборке тестов
graph.o: graph.c graph.h
	$(CC) -DTESTING -c graph.c -o graph.o

u_tests: u_tests.c graph.o $(LIBS)
	$(CC) u_tests.c graph.o $(LIBS) -o u_tests $(CFLAGS) && ./u_tests

test: path_processor
	bash tests/run_tests.sh ./path_processor

test-debug: path_processor
	bash tests/debug.sh ./path_processor

clean:
	rm -f path_processor u_tests graph.o
