#ifndef _QUADTREE_H
#define _QUADTREE_H

#define MAX_BUCKET_SIZE 5

#include <stdint.h>

typedef struct quadtree_bucket quadtree_bucket_t;
typedef struct quadtree_node quadtree_node_t;

quadtree_node_t* quadtree_create(uint32_t, uint32_t, uint32_t, uint32_t);
void quadtree_insert(quadtree_node_t*, void*, uint32_t, uint32_t);
void quadtree_debug(quadtree_node_t*);
void quadtree_query(quadtree_node_t*, uint32_t, uint32_t);

#endif
