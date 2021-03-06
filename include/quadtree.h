#ifndef _QUADTREE_H
#define _QUADTREE_H

#define MAX_BUCKET_SIZE 5

#include <stdint.h>

// Private, Forward Declarations
struct quadtree;
struct quadtree_cursor;

// Query Result
typedef struct quadtree_query_result {
	uint32_t lat;
	uint32_t lng;
	void* data;
} quadtree_query_result_t;

// QuadTree Management
struct quadtree* quadtree_create(uint32_t, uint32_t, uint32_t, uint32_t);
int8_t quadtree_destroy(struct quadtree*);
int8_t quadtree_debug(struct quadtree*);

// QuadTree Insertion
int8_t quadtree_insert(struct quadtree*, void*, uint32_t, uint32_t);

// QuadTree Cursor Management
struct quadtree_cursor* quadtree_cursor_create(struct quadtree*);
int8_t quadtree_cursor_destroy(struct quadtree_cursor*);
int8_t quadtree_cursor_next(struct quadtree_cursor*, struct quadtree_query_result*);
int8_t quadtree_cursor_good(struct quadtree_cursor*);

// QuadTree Query Types
int8_t quadtree_query(struct quadtree*, struct quadtree_cursor*, uint32_t, uint32_t);
int8_t quadtree_range_query(struct quadtree*, struct quadtree_cursor*, uint32_t, uint32_t, uint32_t);

#endif
