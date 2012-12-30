CC=gcc
FLAGS = -Werror -Wall

SRC_DIR=./src
LIB_DIR=./lib
BIN_DIR=./bin
INC_DIR=./include
TEST_DIR=./test

# All reqs for the quadtree impl itself
QUAD_TREE_OBJS=$(LIB_DIR)/quadtree.o

all: $(QUAD_TREE_OBJS)
	@# $(CC) $(FLAGS) $(QUAD_TREE_OBJS) -o $(BIN_DIR)/quadtree

$(LIB_DIR)/quadtree.o: $(SRC_DIR)/quadtree.c $(INC_DIR)/quadtree.h
	$(CC) $(FLAGS) $(SRC_DIR)/quadtree.c -c -o $(LIB_DIR)/quadtree.o
	
clean:
	rm -rf lib/*.o bin/*
