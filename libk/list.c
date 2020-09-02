/* A doubly linked list implementation
 * The implementaion has a sentinel head and tail node
 * In other words, the head an tail aren't included in the list and
 * nodes are inserted after the head/before the tail
 * 
 * TODO: Come back and fix the internals to only use the type functions
 * (e.g. list_set_head)
 * 
 */

#include <stdint.h>
#include <stddef.h>
#include "list.h"

static int node_comparator(list_node_t *node1, list_node_t *node2);

/* LIST INIT FUNCTIONS */

/* initializes a list at list_t *list */
void list_init(list_t *list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->head._struct = NULL;

    list->tail.prev = &list->head;
    list->tail.next = NULL;
    list->tail._struct = NULL;

}

/* LIST MODIFICATION FUNCTIONS */

/* sets the head of a list */
void list_set_head(list_t *list, list_node_t head) {
    list->head.prev = head.prev;
    list->head.next = head.next;
    list->head._struct = head._struct;
}

/* sets the tail of a list */
void list_set_tail(list_t *list, list_node_t tail) {
    list->tail.prev = tail.prev;
    list->tail.next = tail.next;
    list->head._struct = tail._struct;
}

/* inserts node into list */
void list_insert(list_t *list, list_node_t *node) {
   list_insert_front(&list->head, node);
}

/* inserts node into list just before tail */
void list_insert_end(list_node_t *tail, list_node_t *node) {
    tail->prev->next = node;
    node->prev = tail->prev;
    tail->prev = node;
    node->next = tail;
}

/* inserts node into list just after head */
void list_insert_front(list_node_t *head, list_node_t *node) {
   head->next->prev = node;
   node->next = head->next;
   head->next = node;
   node->prev = head;
}

/* inserts node into list in oder determined by comparator */
void list_insert_sorted(list_t *list, list_node_t *node, list_comparator comparator) {
    if (list_isEmpty(list)) {
        list_insert(list, node);
        return;
    }

    list_node_t *curr = list->head.next;

    while (list_hasNext(curr)) {
        if (comparator(curr, node) >= 0) {
            curr->next->prev = node;
            node->next = curr->next;
            curr->next = node;
            node->prev = curr;
            return;
        }

        curr = curr->next;
    }

    node->prev = list->tail.prev;
    node->next = &list->tail;
    list->tail.prev = node;
    return;
}

/* deletes and returns node from list */
list_node_t *list_delete(list_t *list, list_node_t *node) {
    list_node_t *curr = &list->head;
    
    while (curr != NULL && !node_equals(curr, node, node_comparator))
        curr = curr->next;

    if (curr == NULL)
        return curr;
    
    //case where there is only 1 node
    if (curr->next == NULL && curr->prev == NULL) {
        list = NULL;
        return NULL;
    }

    if (curr->prev == NULL) {
        list->head = (*curr->next);
        curr->next->prev = NULL;
    } else if (curr->next == NULL) {
        list->tail = (*curr->prev);
        curr->prev->next = NULL;
    } else {
        curr->next->prev = curr->prev;
        curr->prev->next = curr->next;
    }

    return curr;
}

/* deletes and returns the node just after head, if there is any */
list_node_t *list_pop(list_t *list) {
    if (!list_isEmpty(list))
        return list_delete(list, list->head.next);

    return NULL;
}


/* LIST "GETTER" FUNCTIONS */

/* returns the node just after head, if there is any */
list_node_t *list_peek(list_t *list) {
    if (!list_isEmpty(list))
        return list->head.next;
    
    return NULL;
}

/* checks if node has any node after it 
   only the list tail shouldn't have a node after it */
int list_hasNext(list_node_t *node) {
    if (node->next != NULL )
        return 1;
    return 0;
}

/* checks if list is empty
   a list is empty if only the head and tail are present */
int list_isEmpty(list_t *list) {
    if (list->head.next == &list->tail)
        return 1;
    return 0;
}

/* NODE FUNCTIONS */

/* sets the node specified's next member */
void node_set_next(list_node_t *node, list_node_t *next) {
    node->next = next;
}

/* sets the node specified's prev member */
void node_set_prev(list_node_t *node, list_node_t *prev) {
    node->prev = prev;
}

/* sets the node specified's _struct member */
void node_set_struct(list_node_t *node, void *_struct) {
    node->_struct = _struct;
}

/* default comparator for comparing two nodes */
static int node_comparator(list_node_t *node1, list_node_t *node2) {
    if (node1->next == node2->next && node1->prev == node2->prev)
        if (node1->_struct == node2->_struct)
            return 1;
    return 0;
}

/* checks if two nodes are equal based on comparator 
   both nodes should be in the same list, but it is not required 
   comparator should handle nodes that don't have the same _struct type if it is used by the comparator */
int node_equals(list_node_t *node1, list_node_t *node2, list_comparator comparator) {
    return comparator(node1, node2);
}
