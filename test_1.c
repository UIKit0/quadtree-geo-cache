#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "./quadtree.h"

#define TREE_BOUNDS 10
#define ITERATIONS 20

int main(int argc, char* argv[]) {

	// Seed the random number generator
	srand(time(NULL));

	// Create the quadtree
	struct quadtree* quadtree_ptr = quadtree_create(TREE_BOUNDS, TREE_BOUNDS, TREE_BOUNDS, TREE_BOUNDS);

	int idx, ubound = TREE_BOUNDS*2;
	for(idx = 0; idx < ITERATIONS; ++idx) {
		quadtree_insert(quadtree_ptr, NULL, rand()%(ubound+1), rand()%(ubound+1));
	}

	quadtree_debug(quadtree_ptr);

	fprintf(stderr, "\nQuery Testing\n");
	for(idx = 0; idx < ITERATIONS; ++idx) {
		// Build and execute the query
		struct quadtree_cursor* cursor_ptr = quadtree_cursor_create(quadtree_ptr);
		uint32_t qlat = rand()%(ubound+1), qlng = rand()%(ubound+1);
		quadtree_query(quadtree_ptr, cursor_ptr, qlat, qlng);
		
		// If there was success, print it
		if(quadtree_cursor_good(cursor_ptr)) {
			fprintf(stderr, "\n# Query Lat: %u, Query Lng: %u\n", qlat, qlng);
			do {
				fprintf(stderr, "Result Lat: %u, ", quadtree_cursor_lat(cursor_ptr));
				fprintf(stderr, "Result Lng: %u\n", quadtree_cursor_lng(cursor_ptr));
			} while(quadtree_cursor_next(cursor_ptr));
		}

		// Clean up the cursor
		quadtree_cursor_destroy(cursor_ptr);
	}

	return 0;
}
