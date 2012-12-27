#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "./quadtree.h"

/**
 * Linked List used for node buckets. Uses 24 bytes-per-node
 */
struct quadtree_bucket {

	// Linked List Reqs
	struct quadtree_bucket* next;
	void* bucket_data;
	
	// Item position reqs | 64 - 52 = Excess 12
	uint32_t lat : 26;
	uint32_t lng : 26;

	// Excess bits
	uint32_t _excess_bits : 12;

};

/**
 * Node used to represent a region of the quadtree. Uses 56 bytes-per-node
 */
struct quadtree_node {

	// If this is a leaf node, it points to the object used for data storage
	quadtree_bucket_t* bucket_ptr;

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
};

// Prototypes
static void quadtree_insert_bucket(quadtree_node_t*, quadtree_bucket_t*);
static void quadtree_subdivide(quadtree_node_t* node_ptr);
static void initialize_quadtree_node(quadtree_node_t*);
static quadtree_node_t* find_leaf_by_lat_lng(quadtree_node_t*, uint32_t, uint32_t);

// Function Definitions
////////////////////////////////////////////////////////////////////////////////

void initialize_quadtree_node(quadtree_node_t* node_ptr) {

	// Init the deltas and the centers
	node_ptr->lat = node_ptr->lng = 0;
	node_ptr->delta_lat = node_ptr->delta_lng = 0;

	// Init pointers 
	node_ptr->bucket_ptr = NULL;

	// Needs to be a separate statement to avoid compiler warnings
	node_ptr->children[0] = node_ptr->children[1] = 
	node_ptr->children[2] = node_ptr->children[3] = NULL;

	// Init the flags
	node_ptr->is_leaf_node = 1;

	// Zero the unused bits
	node_ptr->_excess_bits = 0;
}

quadtree_node_t* quadtree_create(uint32_t lat, uint32_t lng, uint32_t delta_lat, uint32_t delta_lng) {
	
	// Allocate the new quadtree pointer
	quadtree_node_t* node_ptr = malloc(sizeof(quadtree_node_t));
	initialize_quadtree_node(node_ptr);

	// Set the default var vals
	node_ptr->lat = lat;
	node_ptr->lng = lng;
	node_ptr->delta_lat = delta_lat;
	node_ptr->delta_lng = delta_lng;

	return node_ptr;
}

void quadtree_subdivide(quadtree_node_t* node_ptr) {
	// Only leaf nodes can be subdivided
	if(!node_ptr->is_leaf_node) { return; }

	// Compute the new deltas
	uint32_t lat = node_ptr->lat, lng = node_ptr->lng;
	uint32_t delta_lat = node_ptr->delta_lat / 2, delta_lng = node_ptr->delta_lng / 2; 

	// If we're at the limit, don't continue to divide
	if(delta_lat <= 0 || delta_lng <= 0) { return; }

	// Allocate and assign the new region nodes
	node_ptr->children[0] = quadtree_create(lat-delta_lat, lng-delta_lng, delta_lat, delta_lng);
	node_ptr->children[1] = quadtree_create(lat+delta_lat, lng-delta_lng, delta_lat, delta_lng);
	node_ptr->children[2] = quadtree_create(lat-delta_lat, lng+delta_lng, delta_lat, delta_lng);
	node_ptr->children[3] = quadtree_create(lat+delta_lat, lng+delta_lng, delta_lat, delta_lng);
	
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

void quadtree_insert_bucket(quadtree_node_t* node_ptr, quadtree_bucket_t* new_bucket_ptr) {
	// Locate the leaf with the helper function
	node_ptr = find_leaf_by_lat_lng(node_ptr, new_bucket_ptr->lat, new_bucket_ptr->lng);

	// Add the new data into the bucket
	quadtree_bucket_t* temp_bucket_ptr = node_ptr->bucket_ptr;
	new_bucket_ptr->next = temp_bucket_ptr;
	node_ptr->bucket_ptr = new_bucket_ptr;
	++node_ptr->bucket_size; 

	// If the node is full, break it up | NOTE - will NOOP if the delta is too small
	if((node_ptr->bucket_size) > MAX_BUCKET_SIZE) {
		quadtree_subdivide(node_ptr);
	}
}

void quadtree_insert(quadtree_node_t* node_ptr, void* data_ptr, uint32_t lat, uint32_t lng) {

	// Ensure the args are good
	assert(node_ptr && node_ptr != NULL);

	// Alloc and init the new bucket node
	quadtree_bucket_t* new_bucket_ptr = malloc(sizeof(quadtree_bucket_t));
	new_bucket_ptr->lat = lat;
	new_bucket_ptr->lng = lng;
	new_bucket_ptr->bucket_data = data_ptr;
	new_bucket_ptr->next = NULL;

	// Wrapper, since this is used elsewhere
	quadtree_insert_bucket(node_ptr, new_bucket_ptr);
}

// Returns all results within a bounding box
void quadtree_range_query(quadtree_node_t* node_ptr, uint32_t lat, uint32_t lng, uint32_t range) {
	quadtree_query(node_ptr, lat, lng);
}

// Returns the leaf node closest to the query params
void quadtree_query(quadtree_node_t* node_ptr, uint32_t lat, uint32_t lng) {
	
	fprintf(stderr, "\nQuery Lat: %u, Query Lng: %u, ", lat, lng);

	node_ptr = find_leaf_by_lat_lng(node_ptr, lat, lng);

	// Print the node
	fprintf(stderr, "\nLat: %u, Lng: %u, ", node_ptr->lat, node_ptr->lng);
	fprintf(stderr, "Lat Delta: %u, Lng Delta: %u, ", node_ptr->delta_lat, node_ptr->delta_lng);
	fprintf(stderr, "Size: %u, Leaf: %u\n", node_ptr->bucket_size, node_ptr->is_leaf_node);
}

// Doesn't need to be super efficient | simply used for debuging
void quadtree_debug(quadtree_node_t* node_ptr) {

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
}