#include <stddef.h>
#include "multiboot.h"
#include "gdt.h"
#include "irq.h"
#include "mmu.h"
#include "printk.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"

void test_MMU(void)
{
    pr_info("\nStarting MMU Test\n");
    void *addr[100];

    printk("\nGetting Page Frames\n");
    for (int i = 0; i < 100; i++) {
        if ((addr[i] = MMU_pf_alloc()) == NULL) {
            pr_err("No more page frames!\n");
            return;
        }
        printk("Page address: %p\n", addr[i]);
    }

    printk("\nFreeing Page Frames\n");
    for (int i = 99; i >= 0; i--) {
        printk("Page address: %p\n", addr[i]);
        MMU_pf_free(addr[i]);
    }

    printk("\nGetting Page Frames\n");
    for (int i = 0; i < 100; i++) {
        if ((addr[i] = MMU_pf_alloc()) == NULL) {
            pr_err("No more page frames!\n");
            return;
        }
        printk("Page address: %p\n", addr[i]);
    }

    printk("\nFreeing Page Frames\n");
    for (int i = 99; i >= 0; i--) {
        printk("Page address: %p\n", addr[i]);
        MMU_pf_free(addr[i]);
    }

    pr_info("MMU Test Completed\n");
}

void stress_MMU(void)
{
    pr_info("Stress Testing MMU\n");
    
    int num_pages = 0;
    intptr_t *pf;  
    while ((pf = (intptr_t*) MMU_pf_alloc()) != NULL) {

        for (size_t j = 0; j < PAGE_SIZE/sizeof(intptr_t); j++)
            pf[j] = (intptr_t) pf; 

        for (size_t j = 0; j < PAGE_SIZE/sizeof(intptr_t); j++) {
            if (pf[j] != (intptr_t) pf) {
                pr_err("[Page %p] Bit pattern on invalid\n", pf);
                return;
            }
        }

        num_pages++;
    }

    pr_info("Total # of Page Frames Allocated: %d\n", num_pages);
    pr_info("No Error Detected!\n");
    pr_info("Stress Testing Complete\n");
}

void kmain(void *tags) 
{
    VGA_init();
    GDT_init();
    IRQ_init();
    SER_init();

    parse_tags(tags);
    MMU_init();
    PS2_init();

    test_MMU();
    stress_MMU();
    
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}