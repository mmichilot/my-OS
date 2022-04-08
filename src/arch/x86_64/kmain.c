#include "vga.h"

void kmain() 
{

#ifdef DEBUG
    int loop = 1;
    while(loop);
#endif
 
    VGA_init();
    VGA_str("Hello, World!\n");
    VGA_bg_color(LIGHT_GRAY);
    VGA_str("Hello, World!\n");
    VGA_bg_color(PINK);
    VGA_str("Hello, World\n");
    
    // Halt indefinitely
    while (1) {
        asm volatile ("hlt");
    }
}