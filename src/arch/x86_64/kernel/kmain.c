#include <stddef.h>
#include "multiboot.h"
#include "gdt.h"
#include "irq.h"
#include "mmu.h"
#include "page_table.h"
#include "printk.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"

void test_virtual_page_alloc(int num_pages) {
    uint64_t *page = (uint64_t*) MMU_alloc_pages(num_pages);
    
    printk("Requested %d pages\n", num_pages);
    printk("Virtual Address: %p\n", page);

    printk("Writing to pages\n");
    for (int i = 0; i < 512*num_pages; i++)
        page[i] = (intptr_t) page;
    printk("Written to pages\n");

    printk("Reading pages\n");
    for (int i = 0; i < 512*num_pages; i++) {
        if (page[i] != (uint64_t) page) {
            pr_err("Error w/ bitpattern");
        }
    }
    printk("All pages read successfully!");
    
    printk("Freeing pages\n");
    MMU_free_pages((void*) page, num_pages);
    
    printk("Attempting to read pages (should page fault)\n");
    for (int i = 0; i < 512*num_pages; i++) {
        if (page[i] != (uint64_t) page)
            printk("Error w/ bitpattern");
    }
}

void kmain(void *tags) 
{
    UNUSED(tags);
    VGA_init();
    GDT_init();
    IRQ_init();
    SER_init();

    parse_tags(tags);
    MMU_init();
    PS2_init();
    PT_init();
    test_virtual_page_alloc(100);
    
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}