/* Defines functionality of default doubly-linked list implementation.
 * NOTE: Needs to be updated to use function pointers */
#ifndef _LIST_H
#define _LIST_H

/* includes */
#include <stddef.h>

/* defines */
#define LIST_ENTRY(ptr, type, member) \
((type *) ((char *) ptr - offsetof(type, member)))

/* structs */
struct list_node {
    struct list_node *next;
    struct list_node *prev;
};

struct list {
    struct list_node head;
    struct list_node tail;
};

/* typedefs */
typedef struct list_node list_node_t;
typedef struct list list_t;

/* functions */

/* init functions */
void list_init(list_t *list);

/* modification functions */
void list_set_head(list_t *list, list_node_t node);
void list_set_tail(list_t *list, list_node_t node);
void list_insert(list_t *list, list_node_t *node);
void list_insert_end(list_node_t *tail, list_node_t *node);
void list_insert_front(list_node_t *head, list_node_t *node);
list_node_t *list_delete(list_t *list, list_node_t *node);
list_node_t *list_pop(list_t *list);

/* "getter" functions */
const list_node_t *list_peek(list_t *list);
const list_node_t *list_get_next(const list_node_t *node);
const list_node_t *list_get_prev(const list_node_t *node);
int list_hasNext(const list_node_t *node);
int list_isEmpty(list_t *list);
size_t list_size(list_t *list);

/* node functions */
void node_set_next(list_node_t *node, list_node_t *next);
void node_set_prev(list_node_t *node, list_node_t *prev);
int node_equals(list_node_t *node1, list_node_t *node2);

#endif