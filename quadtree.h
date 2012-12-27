#ifndef _QUADTREE_H
#define _QUADTREE_H

#define MAX_BUCKET_SIZE 5

#include <stdint.h>

struct quadtree;
struct quadtree_node;

struct quadtree_node* quadtree_create(uint32_t, uint32_t, uint32_t, uint32_t);
void quadtree_insert(struct quadtree_node*, void*, uint32_t, uint32_t);
void quadtree_debug(struct quadtree_node*);
void quadtree_query(struct quadtree_node*, uint32_t, uint32_t);

#endif
