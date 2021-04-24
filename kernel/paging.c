/* This file implements the paging system for the OS. */

/* includes */
#include <stddef.h>
#include <stdio.h>
#include <mem.h>
#include "paging.h"
#include "palloc.h"

/* defines */
#define PGDIR_MASK (0x3FF << 22)
#define PGTABLE_MASK (0x3ff << 12)
#define ENTRY_MASK (0xFFF)

/* globals */
extern char _kernel_start;
extern char _kernel_end;
paddr_t kernel_start = (paddr_t) &_kernel_start;
paddr_t kernel_end = (paddr_t) &_kernel_end;

/* prototypes */
static void enable_paging(paddr_t pg_dir);
static page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create);
static pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr, bool create);
static void init_pde(pde_t *pde, paddr_t addr, bool rw, bool user, bool cache);
static void init_pte(pte_t *pte, paddr_t addr, bool rw, bool user, bool cache);


/* functions */

/* initializes paging system */
int init_paging(page_dir_t **pd) {
    if (pd == NULL)
        return -1;
    
    *pd = (page_dir_t *) PG_ROUND_DOWN(palloc());
    memset(pd, 0, PG_SIZE);

    // identity map first MB
    for (paddr_t pg = PG_SIZE; pg < 1*MB; pg += PG_SIZE)
        paging_kvmap(*pd, pg, pg);
    
    // map kernel to high memory
    for (paddr_t pg = PG_ROUND_DOWN(kernel_start); pg < PG_ROUND_UP(kernel_end); pg += PG_SIZE)
        paging_kmap(*pd, pg);
    
    enable_paging((paddr_t) *pd);
    return 0;
}

static void enable_paging(paddr_t pg_dir) {

}

/* initalizes page tables for a process */
int paging_init(struct process *proc, struct process *proc_parent) {
    return 0;
}

/* maps any virtual address to a physical address */
int paging_map(page_dir_t *pg_dir __attribute__ ((unused)), vaddr_t vaddr __attribute__ ((unused)), paddr_t paddr __attribute__ ((unused))) {
    return -1;
}

/* maps a kernel logical address to a physical address */
int paging_kmap(page_dir_t *pg_dir, paddr_t paddr) {
    paddr_t kstart = (paddr_t) &_kernel_start;
    if (paddr < kstart || paddr >= kstart + (KVADDR_OFFSET - KADDR_OFFSET))
        return -1;
    
    // find page table entry
    page_table_t *page_table = paging_traverse_pgdir(pg_dir, paddr + KADDR_OFFSET, true);
    pte_t *pte = paging_traverse_pgtable(page_table, paddr + KADDR_OFFSET, true);

    // initialize page table entry
    init_pte(pte, paddr, true, false, true);
    return 0;
}

/* maps a kernel virtual address
 * if kvaddr is within the kernel logical address range, paddr is disregarded
 * and kvaddr is mapped to kvaddr - KADDR_OFFSET
 */
int paging_kvmap(page_dir_t *pg_dir, kvaddr_t kvaddr, paddr_t paddr) {
    if (kvaddr >= KADDR_OFFSET && kvaddr < KVADDR_OFFSET)
        return paging_kmap(pg_dir, kvaddr - KADDR_OFFSET);
    
    if (kvaddr < PG_SIZE || PG_ROUND_DOWN(paddr) == 0) // don't allow mapping NULL page
        return -1;
    
    // find page table entry
    page_table_t *page_table = paging_traverse_pgdir(pg_dir, kvaddr, true);
    pte_t *pte = paging_traverse_pgtable(page_table, kvaddr, true);

    // initialize page table entry
    init_pte(pte, paddr, true, false, true);
    return 0;
}

paddr_t *get_current_pgdir() {
    paddr_t *paddr;
    asm volatile ("mov %%cr3, %0" : "=g" (paddr));
    return paddr;
}


static page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create) {
    uint32_t pd_entry = vaddr & PGDIR_MASK;
    pde_t *pde = &pg_dir->tables[pd_entry];

    if (pde->present == 0) {
        if (create == true) {
            // create one
            void *addr = palloc();
            if (addr == NULL)
                return NULL;
            
            init_pde(pde, (paddr_t) addr, true, false, true);
        } else 
            return NULL;
    }

    return (page_table_t *) pde;
}

static pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr, bool create) {
    uint32_t pt_entry = vaddr & PGTABLE_MASK;
    pte_t *pte = &pg_table->entries[pt_entry];

    if (pte->present == 0) {
        if (create == true) {
            // create one
            void *addr = palloc();
            if (addr == NULL)
                return NULL;
            
            init_pte(pte, (paddr_t) addr, true, false, true);
        } else 
            return NULL;
    }

    return pte;
}

static void init_pde(pde_t *pde, paddr_t addr, bool rw, bool user, bool cache) {
    pde->present = 1;
    pde->rw = rw;
    pde->user = user;
    pde->pwt = 0;
    pde->cache = cache;
    pde->accessed = 1;
    pde->addr = addr & ~ENTRY_MASK;
}

static void init_pte(pte_t *pte, paddr_t addr, bool rw, bool user, bool cache) {
    pte->present = 1;   // whether page is present in memory
    pte->rw = rw;   // whether page is read-only or read-write (1)
    pte->user = user;   // whether a page has user or supervisor privilege
    pte->pwt = 0;   // whether a page is write-through (???)
    pte->cache = cache; // whether cache is disabled or not
    pte->accessed = 1;  // whether this page has been accessed
    pte->dirty = 0; // whether this page is dirty (been written to since load)
    pte->pat = 0; // whether PAT is enabled, ignored if PAT isn't supported
    pte->global = 0; // ignored if CR4 is off, whether page is globally mapped (???)
    pte->addr = addr & ~ENTRY_MASK; // physical address of mapped page
}