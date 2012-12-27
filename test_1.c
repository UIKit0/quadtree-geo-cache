#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "./quadtree.h"

#define TREE_BOUNDS 10
#define ITERATIONS 20

int main(int argc, char* argv[]) {

	// Seed the random number generator
	srand(time(NULL));

	int idx, ubound = TREE_BOUNDS*2;
	quadtree_node_t* quadtree_ptr = quadtree_create(TREE_BOUNDS, TREE_BOUNDS, TREE_BOUNDS, TREE_BOUNDS);
	for(idx = 0; idx < ITERATIONS; ++idx) {
		quadtree_insert(quadtree_ptr, NULL, rand()%(ubound+1), rand()%(ubound+1));
	}

	quadtree_debug(quadtree_ptr);

	fprintf(stderr, "\nQuery Testing\n");
	for(idx = 0; idx < ITERATIONS; ++idx) {
		quadtree_query(quadtree_ptr, rand()%(ubound+1), rand()%(ubound+1));
	}

	return 0;
}
