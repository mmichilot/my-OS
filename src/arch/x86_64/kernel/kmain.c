#include "vga.h"
#include "ps2.h"
#include "printk.h"

void printk_test(void)
{
    int val_int = -1;
    short val_short = -1;
    long val_long = -1;
    long long val_long_long = -1;
    char *str = "Hello, World";
    
    printk("Char: %c\n"
           "String: %s\n"
           "Pointer: %p\n",
            *str, str, str);

    printk("int (signed): %d\n"
           "int (unsigned): %u\n"
           "int (unsigned,hex): %x\n",
           val_int, val_int, val_int); 

    printk("short (signed): %hd\n"
           "short (unsigned): %hu\n"
           "short (unsigned,hex): %hx\n",
           val_short, val_short, val_short);

    printk("long (signed): %ld\n"
           "long (unsigned): %lu\n"
           "long (unsigned,hex): %lx\n",
           val_long, val_long, val_long);
    
    printk("long long (signed): %qd\n"
           "long long (unsigned): %qu\n"
           "long long (unsigned,hex): %qx\n",
           val_long_long, val_long_long, val_long_long);
}

void kmain() 
{

#ifdef DEBUG
    int loop = 1;
    while(loop);
#endif

    VGA_init();
    PS2_init();
    while(1) {
       PS2_keyboard_poll();
    }
    //printk_test();
    
    // Halt indefinitely
    pr_err("Halted!\n");
    while (1) {
        asm volatile ("hlt");
    }
}