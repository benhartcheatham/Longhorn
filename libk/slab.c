/* This implementation uses an underlying free list, maybe want to redo with a better data structure? */

#include <stdbool.h>
#include <slab.h>
#include <kerrors.h>

#ifdef TESTS
#include <stdio.h>
#endif

#define SLAB_ROUND_UP(x, size) (((x + size - 1) / size) * size)

static slab_alloc_t default_slab;

struct slab_node {
    void *addr;
    bool free;

    struct slab_node *next;
};

static void *slab_alloc(slab_alloc_t *s, size_t num_slabs) {
    struct slab_node *node = (struct slab_node *) s->data;

    // try and find a region big enough for the allocation
    while (node != NULL) {
        size_t i = 0;

        if (node->free) {
            struct slab_node *temp = node;

            // loop through the next num_slabs free nodes, or until
            // an allocated node is found or the list ends
            while (i < num_slabs && node != NULL) {
                if (node->free) {
                    node = node->next;
                    i++;
                } else
                    break;
            }

            // if the number of free slabs is num_slabs, allocate the region
            if (i == num_slabs) {
                void *mem_addr = temp->addr;

                i = 0;
                while (i < num_slabs) {
                    temp->free = false;
                    temp = temp->next;
                    i++;
                }

                s->free_mem_size -= i * s->slab_size;
                return mem_addr;
            }
        }

        node = node->next;
    }

    // no region that fits the request was found
    return NULL;
}


static int slab_free(slab_alloc_t *s, void *addr, size_t num_slabs) {
    struct slab_node *node = (struct slab_node *) s->data;

    // find the requested node
    while (node != NULL && node->addr != addr)
        node = node->next;
    
    if (node == NULL)
        return -SLAB_FREE_FAIL;
    
    // check the memory area tomake sure
    // entire region is allocated
    struct slab_node *check_node = node;

    size_t i = 0;
    for (i = 0; i < num_slabs && check_node != NULL; ++i, check_node = check_node->next)
        if (check_node->free != false)
            return -SLAB_FREE_FAIL;

    if (i < num_slabs)
        return -SLAB_FREE_FAIL;

    // free region
    for (i = 0; i < num_slabs && node != NULL; ++i, node = node->next)
        node->free = true;
    
    s->free_mem_size += i * s->slab_size;
    return SLAB_SUCC;
}

int slab_init(slab_alloc_t *s, void *mem, size_t mem_size, size_t slab_size, void *aux __attribute__ ((unused))) {
    uint32_t num_slabs = mem_size / slab_size;

    if (mem == NULL)
        return -SLAB_INIT_FAIL;
    

    // make sure there is enough memory for the bookkeeping list
    if (mem_size < sizeof(struct slab_node) * num_slabs)
        return -SLAB_INIT_FAIL;

    // get the starting address for the free memory after the list
    char *starting_addr = mem + SLAB_ROUND_UP((num_slabs * sizeof(struct slab_node)), slab_size);

    // set up the allocator
    struct slab_node *node = mem;
    while (starting_addr < (char *) (mem + mem_size)) {
        // set up the bookkeeping node
        node->addr = starting_addr;
        node->free = true;
        node->next = node + 1;

        node++;
        starting_addr += slab_size;
    }

    // set last node to point to NULL as next
    node--;
    node->next = NULL;

    s->mem = mem;
    s->mem_size = mem_size;
    s->free_mem_size = mem_size - SLAB_ROUND_UP((num_slabs * sizeof(struct slab_node)), slab_size);
    s->slab_size = slab_size;
    s->data = mem;

    s->alloc = slab_alloc;
    s->free = slab_free;
    s->init = NULL;

    return SLAB_SUCC;
}


slab_alloc_t *get_default_slab_allocator() {
    return &default_slab;
}

#ifdef TESTS
    void slab_print_list(slab_alloc_t *s) {
        struct slab_node *node = (struct slab_node *) s->data;
        
        uint32_t i = 0;
        while (node != NULL) {
            printf("node %d: %x | mem: %x | free: %B | next: %x\n", i, node, node->addr, node->free, node->next);
            node = node->next;
            i++;
        }
    }
#endif