#ifndef _LIST_H
#define _LIST_H

/* includes */
#include <stddef.h>

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
typedef struct list_node list_node;
typedef struct list list;

/* Macros */
#define LIST_ENTRY(ptr, type, member) \
((type *) ((char *) ptr - offsetof(type, member)))

/* functions */

/* init functions */
void list_init(list *list);

/* modification functions */
void list_set_head(list *list, list_node node);
void list_set_tail(list *list, list_node node);
void list_insert(list *list, list_node *node);
void list_insert_end(list_node *tail, list_node *node);
void list_insert_front(list_node *head, list_node *node);
list_node *list_delete(list *list, list_node *node);
list_node *list_pop(list *list);

/* "getter" functions */
const list_node *list_peek(list *list);
const list_node *list_get_next(const list_node *node);
const list_node *list_get_prev(const list_node *node);
int list_hasNext(const list_node *node);
int list_isEmpty(list *list);
size_t list_size(list *list);

/* node functions */
void node_set_next(list_node *node, list_node *next);
void node_set_prev(list_node *node, list_node *prev);
int node_equals(list_node *node1, list_node *node2);

#endif