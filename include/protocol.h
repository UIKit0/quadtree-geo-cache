#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdint.h>

struct quadtree_incoming_request;

struct quadtree_incoming_request* incoming_request_create(void);
int8_t incoming_request_destroy(struct quadtree_incoming_request*);
int8_t incoming_request_parse(struct quadtree_incoming_request*, unsigned char*);
int8_t incoming_request_reset(struct quadtree_incoming_request*);
int8_t incoming_request_debug(struct quadtree_incoming_request*);

#endif
