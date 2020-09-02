#ifndef _LIST_H
#define _LIST_H

/* includes */

/* structs */
struct list_node {
    struct list_node *next;
    struct list_node *prev;
    void *_struct;
};

struct list {
    struct list_node head;
    struct list_node tail;
};

/* typedefs */
typedef struct list_node list_node_t;
typedef struct list list_t;
typedef int (list_comparator)(struct list_node *, struct list_node *);

/* functions */

/* init functions */
void list_init(list_t *list);

/* modification functions */
void list_set_head(list_t *list, list_node_t node);
void list_set_tail(list_t *list, list_node_t node);
void list_insert(list_t *list, list_node_t *node);
void list_insert_end(list_node_t *tail, list_node_t *node);
void list_insert_front(list_node_t *head, list_node_t *node);
void list_insert_sorted(list_t *list, list_node_t *node, list_comparator comparator);
list_node_t *list_delete(list_t *list, list_node_t *node);
list_node_t *list_pop(list_t *list);

/* "getter" functions */
list_node_t *list_peek(list_t *list);
int list_hasNext(list_node_t *node);
int list_isEmpty(list_t *list);

/* node functions */
void node_set_next(list_node_t *node, list_node_t *next);
void node_set_prev(list_node_t *node, list_node_t *prev);
void node_set_struct(list_node_t *node, void *_struct);
int node_equals(list_node_t *node1, list_node_t *node2, list_comparator comparator);

#endif