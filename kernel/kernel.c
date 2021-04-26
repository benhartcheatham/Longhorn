/* This file is the starting point for the kernel. It is responsible for initialization of 
 * most kernel subsystems. */

/* includes */
#include <stdbool.h>

/* libc */
#include <stdio.h>

/* Boot */
#include "../boot/multiboot.h"

/* Drivers */
#include "../drivers/bmp.h"
#include "../drivers/vesa.h"
#include "../drivers/line.h"
#include "../drivers/display.h"
#include "../drivers/serial.h"

/* Processes/Threads */
#include "proc.h"
#include "shell.h"

/* Memory Management */
#include "kmalloc.h"
#include "paging.h"

/* Interrupts */
#include "isr.h"

/* Testing */
#ifdef TESTS
    #include "test.h"
#endif

/* defines */
#define FRAMEBUFFER_VIRT_ADDR KVADDR_OFFSET

/* globals */
extern char _binary_assets_longhorn_logo_bmp_start;
uint8_t *header_addr = (uint8_t *) &_binary_assets_longhorn_logo_bmp_start;
bmp_file_header_t header;

/* Summary of version changes:
 * 0.4.0: Added a serial driver in, as well as process features.
 * 0.4.1: Moved print_logo to only run on start in kernel .c
 */
char *version_no = "0.4.1";
static void print_logo();
static void map_framebuffer(multiboot_info_t *mbi, vaddr_t vaddr);

/** Main function of the kernel, starts the kernel and its subsystems. This function
 * returns to a tight loop and then is never scheduled again once completed.
 * 
 * @param mbi: mbi given by GRUB2
 * @param magic: unused
 */
void kmain(multiboot_info_t *mbi, unsigned int magic __attribute__ ((unused))) {
    init_idt();
    init_alloc(mbi);
    init_paging();
    init_processes();

    #ifndef TESTS
        map_framebuffer(mbi, FRAMEBUFFER_VIRT_ADDR);
        display_init((void *) mbi);
        shell_init();
    #else
        init_testing(true);
        RUN_ALL_TESTS(NULL);
    #endif

    #ifndef TESTS
        print_logo();
        kprintf("\nWelcome to Longhorn!\nVersion no.: %s\nType <help> for a list of commands.\n> ", version_no);
        
    #endif

    //shouldn't run more than once
    while (1) {
        thread_yield();
    }
}

#ifndef TESTS
/** prints the logo of the correpsonding size to the screen
   logo sizes are defined in terminal.h */
static void print_logo() {
    read_bmp_header(header_addr, &header);
    bmp_change_color(&header, 0xFFFFFF, 0x0);
    get_default_dis_driver()->dis_clear();
    draw_bmp_data(&header, 10, get_default_dis_driver()->dis_gety() * FONT_HEIGHT + 10);
    get_default_dis_driver()->dis_setcur(0, (header.info_header.height / FONT_HEIGHT) + 1);
}
#endif
static void map_framebuffer(multiboot_info_t *mbi, vaddr_t vaddr) {
    uint32_t bytespp = mbi->framebuffer_bpp / 8;
    for (uint32_t i = 0; i < PG_ROUND_UP(mbi->framebuffer_height * mbi->framebuffer_width * bytespp); i += 4096) {
        paging_kvmap(get_current_pgdir(), vaddr + i, (paddr_t) mbi->framebuffer_addr + i);
    }
    
    mbi->framebuffer_addr = vaddr;
}
