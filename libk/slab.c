/* Implementation of a slab allocator. This implementation uses pointer indexing into an 
 * array of slabs at the beginning of the slab allocator memory */

/* includes */
#include <stdbool.h>
#include <slab.h>
#include <kerrors.h>

#ifdef TESTS
#include <stdio.h>
#endif

/* defines */
#define SLAB_ROUND_UP(x, size) (((x + size - 1) / size) * size)

/* globals */
static slab_alloc_t default_slab;

/* structs */
struct slab {
    void *addr;
    bool free;
};

/* functions */

/** allocates num_slabs from slab allocator s
 * 
 * @param s: slab allocator to allocate from
 * @param num_slabs: number of slabs to allocate
 *
 * @return address of allocated slab(s) or NULL if no region was found
 */
static void *slab_alloc(slab_alloc_t *s, size_t num_slabs) {
    struct slab *slab = (struct slab *) s->data;
    size_t total_num_slabs = ((s->mem_size / s->slab_size) * s->slab_size / sizeof(struct slab));

    // try and find a region big enough for the allocation
    size_t i = 0;
    while (i < total_num_slabs) {

        if (slab->free) {
            struct slab *temp = slab;
            size_t j = 0;
            // loop through the next num_slabs free nodes, or until
            // an allocated node is found or the list ends
            while (j < num_slabs) {
                if (slab->free) {
                    j++;
                    i++;
                    slab++;
                } else
                    break;
            }

            // if the number of free slabs is num_slabs, allocate the region
            if (j == num_slabs) {
                void *mem_addr = temp->addr;

                j = 0;
                while (j < num_slabs) {
                    temp->free = false;
                    temp++;
                    j++;
                }

                s->free_mem_size -= j * s->slab_size;
                return mem_addr;
            }
        }

        slab++;
        i++;
    }

    // no region that fits the request was found
    return NULL;
}

/** frees a previous allocation made by slab allocator s
 * 
 * @param s: slab allocator to free from
 * @param addr: address of previous allocation
 * @param num_slabs: size of previous allocation
 * 
 * @return  -SLAB_FREE_FAIL if free fails, SLAB_SUCC otherwise
 */
static int slab_free(slab_alloc_t *s, void *addr, size_t num_slabs) {
    struct slab *slab = (struct slab *) s->data;
    size_t total_num_slabs = ((s->mem_size / s->slab_size) * s->slab_size / sizeof(struct slab));

    // find the requested node
    size_t i = 0;
    while (slab->addr != addr && i < total_num_slabs)
        slab++;
    
    if (i == total_num_slabs)
        return -SLAB_FREE_FAIL;
    
    // check the memory area tomake sure
    // entire region is allocated
    struct slab *check_slab = slab;

    i = 0;
    for (i = 0; i < num_slabs; i++, check_slab++)
        if (check_slab->free != false)
            return -SLAB_FREE_FAIL;

    if (i < num_slabs)
        return -SLAB_FREE_FAIL;

    // free region
    for (i = 0; i < num_slabs; i++, slab++)
        slab->free = true;
    
    s->free_mem_size += i * s->slab_size;
    return SLAB_SUCC;
}


/** Initializes a new slab allocator at memory location s.
 * NOTE: slab_size must be greater than or equal to the sizeof(struct slab_node), otherwise
 *       the allocator won't initialize properly 
 * 
 * @param s: pointer to slab allocator struct to initialize
 * @param mem: memory for s to allocate from
 * @param mem_size: size of memory for s to allocate from
 * @param slab_size: size of each slab in s
 * @param aux: unused
 * 
 * @return -SLAB_INIT_FAIL on failure to initialize, SLAB_SUCC otherwise
 */
int slab_init(slab_alloc_t *s, void *mem, size_t mem_size, size_t slab_size, void *aux __attribute__ ((unused))) {
    uint32_t num_slabs = mem_size / slab_size;

    if (slab_size < sizeof(struct slab))
        return -SLAB_INIT_FAIL;
    
    if (mem == NULL && mem_size < sizeof(struct slab) * num_slabs)
        return -SLAB_INIT_FAIL;
    
    char *starting_addr = mem + ((num_slabs * slab_size) / sizeof(struct slab));
    // set up the allocator
    struct slab *slab = mem;
    while (starting_addr < (char *) (mem + mem_size)) {
        // set up the bookkeeping node
        slab->addr = starting_addr;
        slab->free = true;

        slab++;
        starting_addr += slab_size;
    }

    s->mem = mem;
    s->mem_size = mem_size;
    s->free_mem_size = mem_size - ((num_slabs * slab_size) / sizeof(struct slab));  // this may be wrong
    s->slab_size = slab_size;
    s->data = mem;

    s->alloc = slab_alloc;
    s->free = slab_free;
    s->init = NULL;

    return SLAB_SUCC;
}

/** gets a pointer to the default slab allocator
 * 
 * @return jpointer to the default slab allocator
 */
slab_alloc_t *get_default_slab_allocator() {
    return &default_slab;
}

#ifdef TESTS
/** prints the underlying linked list of slab allocator s
 * 
 * @param s: slab allocator to print
 */
void slab_print_list(slab_alloc_t *s) {
    struct slab *slab = (struct slab *) s->data;
    uint32_t num_slabs = ((s->mem_size / s->slab_size) * s->slab_size / sizeof(struct slab));
    
    kprintf("sizeof(struct slab): %d\n", sizeof(struct slab));
    kprintf("s->mem_size: %d | s->slab_size: %d | s->free_mem_size: %d\n", s->mem_size, s->slab_size, s->free_mem_size);
    kprintf("number of slabs total: %d\n", num_slabs);
    uint32_t i = 0;
    while (i < num_slabs) {
        kprintf("slab %d: %x | mem: %x | free: %B\n", i, slab, slab->addr, slab->free);
        slab++;
        i++;
    }
}
#endif