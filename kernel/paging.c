/* This file implements the paging system for the OS. */

/* includes */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <mem.h>
#include "paging.h"
#include "palloc.h"
#include "isr.h"

/* defines */
#define PG_SIZE_BITS 12
#define PG_SHIFTL(x) ((paddr_t) x << PG_SIZE_BITS)
#define PG_SHIFTR(x) ((paddr_t) x >> PG_SIZE_BITS)
#define ENTRY_MASK (0x3FF)

/* globals */
extern char _kernel_start;
extern char _kernel_end;
paddr_t kernel_start = (paddr_t) &_kernel_start;
paddr_t kernel_end = (paddr_t) &_kernel_end;

/* prototypes */
static void enable_paging(page_dir_t *pg_dir);
// static page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create);
// static pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr, bool create);
static void init_pde(pde_t *pde, paddr_t addr, bool rw, bool user, bool cache);
static void init_pte(pte_t *pte, paddr_t addr, bool rw, bool user, bool cache);


/* functions */

/* initializes paging system */
int init_paging(page_dir_t **pd) {

    disable_interrupts();
    *pd = (page_dir_t *) palloc();
    memset(*pd, 0, sizeof(page_dir_t));

    // identity map first MB
    for (paddr_t pg = PG_SIZE; pg < 3*MB; pg += PG_SIZE)
       paging_kvmap(*pd, pg, pg);
    
    // map kernel to high memory
    for (paddr_t pg = PG_ROUND_DOWN(kernel_start); pg < 200*PG_SIZE; pg += PG_SIZE)
        paging_kmap(*pd, pg);

    init_pde(&(*pd)->tables[1023], (paddr_t) *pd, true, false, false);

    enable_paging(*pd);
    return 0;
}

static void enable_paging(page_dir_t *pg_dir) {
    //kprintf("pg_dir: %x cr3: %x\n", pg_dir, cr3);
    // uint32_t cr0;
    // asm volatile("mov %%cr0, %0": "=r"(cr0));
    // cr0 |= 0x80000000; // Enable paging!
    // asm volatile("mov %0, %%cr0":: "r"(cr0));
    asm volatile("mov %0, %%cr3 \n\t\
                  mov %%cr0, %%ebx \n\t\
                  or $0x80000001, %%ebx \n\t\
                  mov %%ebx, %%cr0 \n\t": : "r" (pg_dir));
}

/* initalizes page tables for a process */
int paging_init(page_dir_t *proc __attribute__ ((unused)), page_dir_t *proc_parent __attribute__ ((unused))) {
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
    if (page_table == NULL)
        return -1;
    
    pte_t *pte = paging_traverse_pgtable(page_table, paddr + KADDR_OFFSET, true, paddr);
    if (pte == NULL)
        return -1;
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
    if (page_table == NULL)
        return -1;
    
    pte_t *pte = paging_traverse_pgtable(page_table, kvaddr, true, paddr);
    if (pte == NULL)
        return -1;

    return 0;
}

page_dir_t *get_current_pgdir() {
    page_dir_t *paddr;
    asm volatile ("mov %%cr3, %0" : "=g" (paddr));
    return paddr;
}


page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create) {
    uint32_t pde_index = (vaddr >> 22) & ENTRY_MASK;    // index in pg_dir tables array
    pde_t *pde = &pg_dir->tables[pde_index];    // pointer to table in pg_dir tables array

    if(!pde->present) {
        if (create) {
            void *paddr = palloc();
            
            memset((void *)paddr, 0, sizeof(page_table_t));
            init_pde(pde, (paddr_t) paddr, true, false, true);
        } else {
            return NULL;
        }
    }
    
    return (page_table_t *) PG_SHIFTL(pde->table_addr);
}

pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr, bool create, paddr_t paddr) {
    uint32_t pte_index = (vaddr >> 12) & ENTRY_MASK;    // index in pg_table entries array
    pte_t *pte = &pg_table->entries[pte_index]; // pointer to entry in pg_table entries array

    if (!pte->present) {
        if (create) {
            memset(pte, 0, sizeof(pte_t));
            init_pte(pte, paddr, true, false, true);
        } else {
            return NULL;
        }
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
    pde->table_addr = PG_SHIFTR(addr);
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
    pte->addr = PG_SHIFTR(addr); // physical address of mapped page
}