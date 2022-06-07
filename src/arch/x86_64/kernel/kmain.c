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
    
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}