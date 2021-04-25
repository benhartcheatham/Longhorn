/* This file implements the paging system for the OS. Look to paging.h to see
 * specifics of the system */

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
static page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create);
static pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr);
static page_table_t *paging_create_pgtable(page_dir_t *pg_dir, vaddr_t vaddr);
static pte_t *paging_create_pte(page_table_t *pg_table, vaddr_t vaddr, paddr_t paddr);
static void init_pde(pde_t *pde, paddr_t addr, bool rw, bool user, bool cache);
static void init_pte(pte_t *pte, paddr_t addr, bool rw, bool user, bool cache);


/* functions */

/** initializes paging system 
 * 
 * @return 0 on success, -1 otherwise
 */
int init_paging() {

    disable_interrupts();
    page_dir_t *pd = (page_dir_t *) palloc();
    memset(pd, 0, sizeof(page_dir_t));

    // identity map first MB
    for (paddr_t pg = PG_SIZE; pg < 3*MB; pg += PG_SIZE)
       paging_kvmap(pd, pg, pg);
    
    // map kernel to high memory
    for (paddr_t pg = PG_ROUND_DOWN(kernel_start); pg < KVADDR_OFFSET; pg += PG_SIZE)
        paging_kmap(pd, pg);

    // map last page table to point to page directory
    page_table_t *dir_table = paging_create_pgtable(pd, 1023*4*MB); // address of last page table
    for (uint32_t i = 0; i < (sizeof(page_table_t) / sizeof(pte_t)) - 1; i++) { // the minus one is so that we don't map this table again
        pte_t *pte = &dir_table->entries[i];
        init_pte(pte, (paddr_t) PG_SHIFTL(pd->tables[i].table_addr), true, false, true);
    }

    enable_paging(pd);
    return 0;
}

/** enables the paging feature
 * 
 * @param pg_dir: physical address of page directory to enable
 *                paging with
 */
static void enable_paging(page_dir_t *pg_dir) {
    asm volatile("mov %0, %%cr3 \n\t\
                  mov %%cr0, %%ebx \n\t\
                  or $0x80000001, %%ebx \n\t\
                  mov %%ebx, %%cr0 \n\t": : "r" (pg_dir));
}

/** !!! UNIMPLEMENTED !!!
 * initalizes page tables for a process 
 * 
 * @param proc: process to initialize page tables of
 * 
 * @return 0 on success, -1 otherwise
*/
int paging_init(page_dir_t *proc __attribute__ ((unused))) {
    return -1;
}

/** !!! UNIMPLEMENTED !!!
 *  maps a user virtual address to a physical address 
 * will not map the NULL page or any address above KADDR_OFFSET
 * 
 * @param pg_dir: page directory to map page in
 * @param vaddr: user virtual address to map
 * @param paddr: physical address to map vaddr to
 * 
 * @return 0 on success, -1 otherwise
 */
int paging_map(page_dir_t *pg_dir __attribute__ ((unused)), vaddr_t vaddr __attribute__ ((unused)), paddr_t paddr __attribute__ ((unused))) {
    return -1;
}

/** maps a kernel logical address to a physical address 
 * kernel logical addresses are equivalent to a virtual address of
 * physical address + KADDR_OFFSET
 * 
 * @param pg_dir: page directory to map kernel logical address into
 * @param paddr: physical address to map paddr + KADDR_OFFSET to
 * 
 * @return 0 on success, -1 on failure
 */
int paging_kmap(page_dir_t *pg_dir, paddr_t paddr) {
    paddr_t kstart = (paddr_t) &_kernel_start;
    if (paddr < kstart || paddr >= kstart + (KVADDR_OFFSET - KADDR_OFFSET))
        return -1;
    
    // find page table entry
    page_table_t *page_table = paging_traverse_pgdir(pg_dir, paddr + KADDR_OFFSET, true);
    if (page_table == NULL)
        return -1;
    
    pte_t *pte = paging_traverse_pgtable(page_table, paddr + KADDR_OFFSET);
    if (pte == NULL)
        paging_create_pte(page_table, paddr + KADDR_OFFSET, paddr);
    
    if (pte == NULL)
        return -1;
    
    return 0;
}

/** maps a virtual address with kernel permissions
 * 
 * if kvaddr is within the kernel logical address range, paddr is disregarded
 * and kvaddr is mapped to kvaddr - KADDR_OFFSET
 * 
 * @param pg_dir: page directory to map page in
 * @param kvaddr: kernel virtual address to map
 * @param paddr: physical address to map kvaddr to
 * 
 * @return 0 on success, -1 on failure
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
    
    pte_t *pte = paging_traverse_pgtable(page_table, kvaddr);
    if (pte == NULL)
        paging_create_pte(page_table, kvaddr, paddr);
    
    if (pte == NULL)
        return -1;

    return 0;
}

/** returns a pointer to the PHYSICAL address of 
 * the current page directory
 * 
 * @return pointer to physical address of page directory
 */
page_dir_t *get_current_pgdir() {
    page_dir_t *paddr;
    asm volatile ("mov %%cr3, %0" : "=g" (paddr));
    return paddr;
}

/** finds the page table that contains vaddr in pg_dir, if it exists
 * 
 * @param pg_dir: page directory to look for vaddr in
 * @param vaddr: virtual address to lookup
 * @param create: whether to create the table containing vaddr if it does not exist
 * 
 * @return a pointer to the physical address of the page table containing vaddr, 
 *         NULL if it doesn't exist and create is false
 */
static page_table_t *paging_traverse_pgdir(page_dir_t *pg_dir, vaddr_t vaddr, bool create) {
    uint32_t pde_index = (vaddr >> 22) & ENTRY_MASK;    // index in pg_dir tables array
    pde_t *pde = &pg_dir->tables[pde_index];    // pointer to table in pg_dir tables array

    if(!pde->present) {
        if (create)
            return paging_create_pgtable(pg_dir, vaddr);
        else
            return NULL;
    }
    
    return (page_table_t *) PG_SHIFTL(pde->table_addr);
}

/** finds the page table entry that maps to vaddr
 * 
 * @param pg_table: page table to look for vaddr in
 * @param vaddr: virtual address mapped to pte
 * 
 * @return pte mapped to vaddr, NULL if it doesn't exist
 */
static pte_t *paging_traverse_pgtable(page_table_t *pg_table, vaddr_t vaddr) {
    uint32_t pte_index = (vaddr >> 12) & ENTRY_MASK;    // index in pg_table entries array
    pte_t *pte = &pg_table->entries[pte_index]; // pointer to entry in pg_table entries array

    if (!pte->present)
        return NULL;

    return pte;
}

/** creates the page table that contains vaddr
 * 
 * @param pg_dir: page directory to create page table in
 * @param vaddr: virtual address corresponding to page table to create
 * 
 * @return pointer to physical address of created page_table, NULL if creation failed
 */
static page_table_t *paging_create_pgtable(page_dir_t *pg_dir, vaddr_t vaddr) {
    uint32_t pde_index = (vaddr >> 22) & ENTRY_MASK;    // index in pg_dir tables array
    pde_t *pde = &pg_dir->tables[pde_index];    // pointer to table in pg_dir tables array

    if (!pde->present) {
        void *paddr = palloc();
            
        memset((void *)paddr, 0, sizeof(page_table_t));
        init_pde(pde, (paddr_t) paddr, true, false, true);
    }

    return (page_table_t *) PG_SHIFTL(pde->table_addr);
}

/** creates the pte at vaddr that maps to paddr
 * 
 * @param pg_table: page table to create pte in
 * @param vaddr: virtual address corresponding to pte to create
 * @param paddr: physical address to map new pte to
 * 
 * @return pointer to physcial address of created pte, NULL if creation failed
 */
static pte_t *paging_create_pte(page_table_t *pg_table, vaddr_t vaddr, paddr_t paddr) {
    uint32_t pte_index = (vaddr >> 12) & ENTRY_MASK;    // index in pg_table entries array
    pte_t *pte = &pg_table->entries[pte_index]; // pointer to entry in pg_table entries array

    if (!pte->present) {
        memset(pte, 0, sizeof(pte_t));
        init_pte(pte, paddr, true, false, true);
    }

    return pte;
}

/** initializes the fields of a page directory entry
 * 
 * @param pde: pointer to address in page directory of pde
 * @param addr: physical address of pde
 * @param rw: whether the pde is read/write or read-only
 * @param user: whether the pde is user accessible
 * @param cache: whether the pde is cacheable
 */
static void init_pde(pde_t *pde, paddr_t addr, bool rw, bool user, bool cache) {
    pde->present = 1;
    pde->rw = rw;
    pde->user = user;
    pde->pwt = 0;
    pde->cache = cache;
    pde->accessed = 1;
    pde->table_addr = PG_SHIFTR(addr);
}

/** initializes the fields of a page table entry
 * 
 * @param pde: pointer to address in page table of pte
 * @param addr: physical address of page
 * @param rw: whether the page is read/write or read-only
 * @param user: whether the page is user accessible
 * @param cache: whether the page is cacheable
 */
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