CC = gcc -ggdb -std=c99 -Wall -Werror -O3
OUT = listdir

file_stats.o:
	$(CC) -c file_stats.c

all: clean file_stats.o
	$(CC) -o $(OUT) listdir.c file_stats.o

clean:
	rm -f *.o
	rm -f $(OUT)

install: clean all

run: install
	./$(OUT)

rerun: all
	./$(OUT)

valgrind: clean all
	valgrind -v --leak-check=full ./$(OUT)