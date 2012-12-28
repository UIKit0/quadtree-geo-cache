#ifndef _QUADTREE_H
#define _QUADTREE_H

#define MAX_BUCKET_SIZE 5

#include <stdint.h>

struct quadtree;

struct quadtree* quadtree_create(uint32_t, uint32_t, uint32_t, uint32_t);
void quadtree_insert(struct quadtree*, void*, uint32_t, uint32_t);
void quadtree_debug(struct quadtree*);
void quadtree_query(struct quadtree*, uint32_t, uint32_t);
void quadtree_range_query(struct quadtree*, uint32_t, uint32_t, uint32_t);

#endif
