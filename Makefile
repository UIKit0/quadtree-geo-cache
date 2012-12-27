CC=gcc
FLAGS = -Werror -Wall

all: test_1.o quadtree.o
	$(CC) $(FLAGS) test_1.o quadtree.o -o test_1

test_1.o: test_1.c
	$(CC) $(FLAGS) test_1.c -c

quadtree.o: quadtree.c quadtree.h
	$(CC) $(FLAGS) quadtree.c -c
	
clean:
	rm -rf *.o test_1
