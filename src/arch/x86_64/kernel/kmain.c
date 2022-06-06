#include <stddef.h>
#include "multiboot.h"
#include "gdt.h"
#include "irq.h"
#include "mmu.h"
#include "printk.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"

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