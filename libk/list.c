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

static int node_comparator(list_node *node1, list_node *node2);

/* LIST INIT FUNCTIONS */

/* initializes a list at list_t *list */
void list_init(list *list) {
    list->head.prev = NULL;
    list->head.next = &list->tail;

    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

/* LIST MODIFICATION FUNCTIONS */

/* sets the head of a list */
void list_set_head(list *list, list_node head) {
    list->head.prev = head.prev;
    list->head.next = head.next;
}

/* sets the tail of a list */
void list_set_tail(list *list, list_node tail) {
    list->tail.prev = tail.prev;
    list->tail.next = tail.next;
}

/* inserts node into list */
void list_insert(list *list, list_node *node) {
   list_insert_front(&list->head, node);
}

/* inserts node into list just before tail */
void list_insert_end(list_node *tail, list_node *node) {
    tail->prev->next = node;
    node->prev = tail->prev;
    tail->prev = node;
    node->next = tail;
}

/* inserts node into list just after head */
void list_insert_front(list_node *head, list_node *node) {
   head->next->prev = node;
   node->next = head->next;
   head->next = node;
   node->prev = head;
}

/* inserts node into list in oder determined by comparator */
void list_insert_sorted(list *list, list_node *node, list_comparator comparator) {
    if (list_isEmpty(list)) {
        list_insert(list, node);
        return;
    }

    list_node *curr = list->head.next;

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
list_node *list_delete(list *list, list_node *node) {
    list_node *curr = &list->head;
    
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
list_node *list_pop(list *list) {
    if (!list_isEmpty(list))
        return list_delete(list, list->head.next);

    return NULL;
}


/* LIST "GETTER" FUNCTIONS */

/* returns a const pointer to the node just after head, if there is any *
   should be used in conjunction with list_get_next/list_get_prev to iterate over a list */
const list_node *list_peek(list *list) {
    if (!list_isEmpty(list))
        return list->head.next;
    
    return NULL;
}

/* returns a const pointer to the next node in the linked list
   should be used in conjunction with list_peek/list_get_prev to iterate over a list */
const list_node *list_get_next(const list_node *node) {
    return node->next;
}

/* returns a const pointer to the previous node in the linked list
   should be used in conjunction with list_peek/list_get_next to iterate over a list */
const list_node *list_get_prev(const list_node *node) {
    return node->prev;
}

/* checks if node has any node after it 
   only the list tail shouldn't have a node after it */
int list_hasNext(const list_node *node) {
    if (node->next != NULL )
        return 1;
    return 0;
}

/* checks if list is empty
   a list is empty if only the head and tail are present */
int list_isEmpty(list *list) {
    if (list->head.next == &list->tail)
        return 1;
    return 0;
}

/* NODE FUNCTIONS */

/* sets the node specified's next member */
void node_set_next(list_node *node, list_node *next) {
    node->next = next;
}

/* sets the node specified's prev member */
void node_set_prev(list_node *node, list_node *prev) {
    node->prev = prev;
}

/* default comparator for comparing two nodes */
static int node_comparator(list_node *node1, list_node *node2) {
    if (node1->next == node2->next && node1->prev == node2->prev)
        return 1;
    return 0;
}

/* checks if two nodes are equal based on comparator 
   both nodes should be in the same list, but it is not required 
   comparator should handle nodes that don't have the same _struct type if it is used by the comparator */
int node_equals(list_node *node1, list_node *node2, list_comparator comparator) {
    return comparator(node1, node2);
}
