#include <stddef.h>
#include "gdt.h"
#include "printk.h"
#include "irq.h"
#include "drivers/vga.h"
#include "drivers/ps2.h"
#include "drivers/serial.h"

void kmain(void) 
{
    VGA_init();
    GDT_init();
    IRQ_init();
    SER_init();
    PS2_init();
    
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}