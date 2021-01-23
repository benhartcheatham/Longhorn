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
#include "../libc/stdio.h"

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

/* deletes and returns node from list */
list_node *list_delete(list *list, list_node *node) {
    list_node *curr = &list->head;
    
    while (curr != NULL && curr != node)
        curr = curr->next;

    if (curr == NULL)
        return NULL;

    // don't allow deletion of the head or tail since they don't "exist"
    if (curr == &list->head || curr == &list->tail)
        return NULL;
    
    // make new links
    curr->prev->next = curr->next;
    curr->next->prev = curr->prev;

    // might want to null out old links for safety, have to think about that
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

size_t list_size(list *list) {
    list_node *curr = list->head.next;

    size_t size = 0;
    while (curr != NULL && list_hasNext(curr)) {
        curr = curr->next;
        size++;
    }

    return size;
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

/* checks if two nodes are equal based on comparator 
   both nodes should be in the same list, but it is not required 
   comparator should handle nodes that don't have the same _struct type if it is used by the comparator */
int node_equals(list_node *node1, list_node *node2) {
    return node1 == node2;
}
