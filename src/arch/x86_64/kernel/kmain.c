#include <stddef.h>
#include "multiboot.h"
#include "gdt.h"
#include "irq.h"
#include "mmu.h"
#include "page_table.h"
#include "printk.h"
#include "kmalloc.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"

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
    kmalloc_init();

    printk("\nRequesting 10000 bytes\n");
    char *block = (char*) kmalloc(10000);


    printk("\nWriting to all bytes...\n");
    for (int i = 0; i < 10000; i++)
        block[i] = i % 255;

    printk("\nVerifying all 10000 bytes\n");
    for (int i = 0; i < 10000; i++) {
        if (block[i] != (char) (i % 255))
            printk("[%d] Wrong value\n", i);
    }

    printk("\nFreeing memory\n");
    kfree(block);

    // char *blocks[100];
    // char *blocks2[100];
    // char *blocks3[100];
    // char *blocks4[100];
    // char *blocks5[100];
    char *blocks6[100];

    printk("\nRequesting 100 blocks from each pool\n");
    for (int i = 0; i < 100; i++) {
        // blocks[i] = (char*) kmalloc(32);
        // blocks2[i] = (char*) kmalloc(64);
        // blocks3[i] = (char*) kmalloc(128);
        // blocks4[i] = (char*) kmalloc(256);
        // blocks5[i] = (char*) kmalloc(512);
        blocks6[i] = (char*) kmalloc(1024);
    }

    printk("\nFreeing all 100 blocks from each pool\n");
    for (int i = 0; i < 100; i++) {
        // kfree(blocks[i]);
        // kfree(blocks2[i]);
        // kfree(blocks3[i]);
        // kfree(blocks4[i]);
        // kfree(blocks5[i]);
        kfree(blocks6[i]);
    }
     
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}