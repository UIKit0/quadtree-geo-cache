#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../include/quadtree.h"

// Forward Declarations
struct quadtree_node;
struct quadtree_bucket;

/**
 * Client facing handle for the quadtree
 */
typedef struct quadtree {
	struct quadtree_node* root_node_ptr;
} quadtree_t;

/**
 * Cursor for any queries made to the service
 */
typedef struct quadtree_cursor {
	struct quadtree* quadtree_ptr;
	struct quadtree_bucket* bucket_ptr;
} quadtree_cursor_t;

/**
 * Linked List used for node buckets. 
 */
typedef struct quadtree_bucket {

	// Linked List Reqs
	struct quadtree_bucket* next;
	void* bucket_data;
	
	// Item position reqs | 64 - 52 = Excess 12
	uint32_t lat : 26;
	uint32_t lng : 26;

	// Excess bits
	uint32_t _excess_bits : 12;

} quadtree_bucket_t;

/**
 * Node used to represent a region of the quadtree. 
 */
typedef struct quadtree_node {

	// If this is a leaf node, it points to the object used for data storage
	struct quadtree_bucket* bucket_ptr;

	// Lat-Lng bits and delta bits : Total 128 - 104 = Excess 24
	uint32_t lat				: 26;
	uint32_t lng				: 26;
	uint32_t delta_lat	: 26;
	uint32_t delta_lng	: 26;

	// 24 - 16 = Excess 8
	uint16_t bucket_size : 16; 

	// Bit Flags 8 - 1 = Excess 7
	uint8_t is_leaf_node : 1;

	// Excess Bits
	uint32_t _excess_bits : 7;

	/**
	 * Children node pointers. The regions are defined in the picture.
	 *
	 * ---------------
	 * |      |      |
	 * |  01  |  11  |
	 * |      |      |
	 * |-------------|
	 * |      |      |
	 * |  00  |  10  |
	 * |      |      |
	 * ---------------
	 */

	struct quadtree_node* children[4];
} quadtree_node_t;

// Prototypes
static quadtree_node_t* quadtree_root_create(uint32_t, uint32_t, uint32_t, uint32_t);
static int8_t quadtree_insert_bucket(quadtree_node_t*, quadtree_bucket_t*);
static int8_t quadtree_subdivide(quadtree_node_t* node_ptr);
static void quadtree_node_init(quadtree_node_t*);
static quadtree_node_t* find_leaf_by_lat_lng(quadtree_node_t*, uint32_t, uint32_t);

/*******************************************************************************
 * Quad Tree Creation                                                          *
 *******************************************************************************/

void quadtree_node_init(quadtree_node_t* node_ptr) {

	// Init the deltas and the centers
	node_ptr->lat = node_ptr->lng = 0;
	node_ptr->delta_lat = node_ptr->delta_lng = 0;

	// Init pointers 
	node_ptr->bucket_ptr = NULL;

	// Needs to be a separate statement (from bucket_ptr) to avoid compiler warnings
	node_ptr->children[0] = node_ptr->children[1] = 
	node_ptr->children[2] = node_ptr->children[3] = NULL;

	// Init the flags
	node_ptr->is_leaf_node = 1;

	// Zero the unused bits
	node_ptr->_excess_bits = 0;
}

quadtree_node_t* quadtree_root_create(uint32_t lat, uint32_t lng, uint32_t delta_lat, uint32_t delta_lng) {

	// Create and init the node
	quadtree_node_t* node_ptr = malloc(sizeof(quadtree_node_t));
	quadtree_node_init(node_ptr);

	// Set the default var vals
	node_ptr->lat = lat;
	node_ptr->lng = lng;
	node_ptr->delta_lat = delta_lat;
	node_ptr->delta_lng = delta_lng;

	return node_ptr;
}

quadtree_t* quadtree_create(uint32_t lat, uint32_t lng, uint32_t delta_lat, uint32_t delta_lng) {
	quadtree_t* quadtree_ptr = malloc(sizeof(quadtree_t));
	quadtree_ptr->root_node_ptr = quadtree_root_create(lat, lng, delta_lat, delta_lng);
	return quadtree_ptr;
}

int8_t quadtree_destroy(quadtree_t* quadtree_ptr) {
	// Iterate through the tree, push the children onto a queue, and delete the root
	return 0;
}

/*******************************************************************************
 * Quad Tree Maintenance                                                       *
 *******************************************************************************/

int8_t quadtree_subdivide(quadtree_node_t* node_ptr) {
	// Only leaf nodes can be subdivided
	if(!node_ptr->is_leaf_node) { return -1; }

	// Compute the new deltas
	uint32_t lat = node_ptr->lat, lng = node_ptr->lng;
	uint32_t delta_lat = node_ptr->delta_lat / 2, delta_lng = node_ptr->delta_lng / 2; 

	// If we're at the limit, don't continue to divide
	if(delta_lat <= 0 || delta_lng <= 0) { return -1; }

	// Allocate and assign the new region nodes
	node_ptr->children[0] = quadtree_root_create(lat-delta_lat, lng-delta_lng, delta_lat, delta_lng);
	node_ptr->children[1] = quadtree_root_create(lat+delta_lat, lng-delta_lng, delta_lat, delta_lng);
	node_ptr->children[2] = quadtree_root_create(lat-delta_lat, lng+delta_lng, delta_lat, delta_lng);
	node_ptr->children[3] = quadtree_root_create(lat+delta_lat, lng+delta_lng, delta_lat, delta_lng);
	
	// Update the leaf-node bit
	node_ptr->is_leaf_node = 0;

	// Move the existing nodes into new buckets
	while(node_ptr->bucket_ptr != NULL) {
		quadtree_bucket_t* bucket_ptr = node_ptr->bucket_ptr;

		node_ptr->bucket_ptr = bucket_ptr->next;
		bucket_ptr->next = NULL; // Ensure that `next` well defined (cleared)

		--node_ptr->bucket_size;
		quadtree_insert_bucket(node_ptr, bucket_ptr);
	}

	return 0;
}

quadtree_node_t* find_leaf_by_lat_lng(quadtree_node_t* node_ptr, uint32_t lat, uint32_t lng) {

	// Assert since this is a static function
	assert(node_ptr && node_ptr != NULL);

	// Iterate until the next node is found
	while(!node_ptr->is_leaf_node) {
		// Get the child id and the pointer for the next node
		uint8_t child_id = (lat > node_ptr->lat) & 0x1;
		child_id |= ((lng > node_ptr->lng) & 0x1) << 1;
		node_ptr = node_ptr->children[child_id];
		// If the current node_ptr isn't a leaf, this should never be NULL
		assert(node_ptr && node_ptr != NULL);
	}

	return node_ptr;
}

/*******************************************************************************
 * Quad Tree Insertion                                                         *
 *******************************************************************************/

int8_t quadtree_insert_bucket(quadtree_node_t* node_ptr, quadtree_bucket_t* new_bucket_ptr) {
	// Locate the leaf with the helper function
	node_ptr = find_leaf_by_lat_lng(node_ptr, new_bucket_ptr->lat, new_bucket_ptr->lng);

	// Add the new data into the bucket
	quadtree_bucket_t* temp_bucket_ptr = node_ptr->bucket_ptr;
	new_bucket_ptr->next = temp_bucket_ptr;
	node_ptr->bucket_ptr = new_bucket_ptr;
	++node_ptr->bucket_size; 

	// If the node is full, break it up. Will NOOP if the delta is too small.
	// Some leaf nodes will excede the MAX_BUCKET_SIZE. Should return an error code in that case
	return (node_ptr->bucket_size > MAX_BUCKET_SIZE) ? quadtree_subdivide(node_ptr) : 0;
}

int8_t quadtree_insert(quadtree_t* quadtree_ptr, void* data_ptr, uint32_t lat, uint32_t lng) {

	// Ensure the args are good
	assert(quadtree_ptr && quadtree_ptr != NULL);

	// Alloc and init the new bucket node
	quadtree_bucket_t* new_bucket_ptr = malloc(sizeof(quadtree_bucket_t));
	new_bucket_ptr->lat = lat;
	new_bucket_ptr->lng = lng;
	new_bucket_ptr->bucket_data = data_ptr;
	new_bucket_ptr->next = NULL;

	// Wrapper, since this is used elsewhere
	return quadtree_insert_bucket(quadtree_ptr->root_node_ptr, new_bucket_ptr);
}

/*******************************************************************************
 * Quad Tree Cursors                                                           *
 *******************************************************************************/

quadtree_cursor_t* quadtree_cursor_create(quadtree_t* quadtree_ptr) {
	// Allocate and init the cursor
	quadtree_cursor_t* cursor_ptr = malloc(sizeof(quadtree_cursor_t*));
	cursor_ptr->bucket_ptr = NULL;

	// Assign the quadtree and acquire the rw-lock
	cursor_ptr->quadtree_ptr = quadtree_ptr;

	return cursor_ptr;
}

int8_t quadtree_cursor_destroy(quadtree_cursor_t* cursor_ptr) {
	// Unlock the rw-lock
	/* ... */

	// Free the allocated cursor
	free(cursor_ptr);
	return 0;
}

int8_t quadtree_cursor_good(quadtree_cursor_t* cursor_ptr) {
	return cursor_ptr->bucket_ptr != NULL;
}

int8_t quadtree_cursor_next(quadtree_cursor_t* cursor_ptr, quadtree_query_result_t* result_ptr) {
	if(!quadtree_cursor_good(cursor_ptr)) {
		return 0;
	} 

	result_ptr->lat = cursor_ptr->bucket_ptr->lat;
	result_ptr->lng = cursor_ptr->bucket_ptr->lng;
	result_ptr->data = cursor_ptr->bucket_ptr->bucket_data;

	cursor_ptr->bucket_ptr = cursor_ptr->bucket_ptr->next;
	return 1;
}

/*******************************************************************************
 * Quad Tree Queries                                                           *
 *******************************************************************************/

// Returns all results within a bounding box - EXPENSIVE
int8_t quadtree_range_query(quadtree_t* quadtree_ptr, quadtree_cursor_t* cursor_ptr, uint32_t lat, uint32_t lng, uint32_t range) {

	// If a non-leaf node's center is in the bounding box, add it to the queue
	// If a leaf node's center is in the bounding box, add it to the result list

	// If the bounds of the result are outside of the bounding box, iterate
	// through the bucket. Else, add the entire bucket to the result set.

	// XXX - For now, simply fall back to a simple query
	return quadtree_query(quadtree_ptr, cursor_ptr, lat, lng);
}

// Returns the leaf node closest to the query params
int8_t quadtree_query(quadtree_t* quadtree_ptr, quadtree_cursor_t* cursor_ptr, uint32_t lat, uint32_t lng) {
	quadtree_node_t* node_ptr = find_leaf_by_lat_lng(quadtree_ptr->root_node_ptr, lat, lng);
	if(node_ptr == NULL) { return -1; }
	cursor_ptr->bucket_ptr = node_ptr->bucket_ptr;
	return 0;
}

// Doesn't need to be super efficient | simply used for debuging
int8_t quadtree_debug(quadtree_t* quadtree_ptr) {
	
	// Temp node pointer
	quadtree_node_t* node_ptr = quadtree_ptr->root_node_ptr;

	// Debug list-queue structure
	typedef struct { void* next; quadtree_node_t* data; } debug_node_t;
	debug_node_t *head_ptr = NULL, *tail_ptr = NULL, *temp_ptr = NULL;

	// Assign initial node
	head_ptr = tail_ptr = malloc(sizeof(debug_node_t));
	head_ptr->data = node_ptr;

	fprintf(stderr, "\nQuadtree State\n");
	while(head_ptr != NULL) {
		// If this isn't a leaf node, add the children to the queue
		if(!head_ptr->data->is_leaf_node) {
			int8_t idx;
			for(idx = 0; idx < 4; ++idx) {
				temp_ptr = malloc(sizeof(debug_node_t));
				temp_ptr->next = NULL;
				temp_ptr->data = head_ptr->data->children[idx];
				tail_ptr->next = temp_ptr;
				tail_ptr = temp_ptr;
			}
		}

		// Grab the head's data, shift off the queue, and free the memory
		quadtree_node_t* data_ptr = head_ptr->data;
		temp_ptr = head_ptr;
		head_ptr = head_ptr->next;
		free(temp_ptr);

		// Print the node
		fprintf(stderr, "\nLat: %u, Lng: %u, ", data_ptr->lat, data_ptr->lng);
		fprintf(stderr, "Lat Delta: %u, Lng Delta: %u, ", data_ptr->delta_lat, data_ptr->delta_lng);
		fprintf(stderr, "Size: %u, Leaf: %u\n", data_ptr->bucket_size, data_ptr->is_leaf_node);

		// Print the bucket
		quadtree_bucket_t* bucket_head_ptr = data_ptr->bucket_ptr;
		while(bucket_head_ptr) {
			fprintf(stderr, "Lat: %u, Lng: %u, ", bucket_head_ptr->lat, bucket_head_ptr->lng);
			int32_t lat_diff = abs(bucket_head_ptr->lat - data_ptr->lat);
			int32_t lng_diff = abs(bucket_head_ptr->lng - data_ptr->lng);
			fprintf(stderr, "Lat Diff: %u, Lng Diff: %u\n", lat_diff, lng_diff);
			bucket_head_ptr = bucket_head_ptr->next;
		}
	}

	return 0;
}
