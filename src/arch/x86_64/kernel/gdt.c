#include <stdint-gcc.h>
#include "gdt.h"
#include "printk.h"

static struct {
    uint32_t :32;       // Segment Limit 1 and Base Address 1 (ignored in long mode)
    uint8_t  :8;        // Base address 2 (ignored in long mode)
    uint8_t access;    
    uint8_t  :4;        // Segment Limit 2 (ignored in long mode)
    uint8_t  flags:4;
    uint8_t  :8;        // Base address 3 (ignored in long mode)
}__attribute__((packed)) GDT[6];

static inline void lgdt(void* base, uint16_t size)
{
    static struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) GDTR;

    GDTR.length = size;
    GDTR.base = base;

    asm volatile ( "lgdt %0" : : "m"(GDTR) );
}

void GDT_init(void)
{
    pr_info("Initializing GDT\n");

    GDT[KERNEL_CS].access = (EXEC | TYPE_CS | DPL_KERNEL | PRESENT);
    GDT[KERNEL_CS].flags = (LONG_MODE);

    GDT[USER_CS].access = (EXEC | TYPE_CS | DPL_USER | PRESENT);
    GDT[USER_CS].flags = (LONG_MODE);

    GDT[USER_DS].access = (TYPE_DS | PRESENT);
    
    lgdt((void*)GDT, sizeof(GDT));

    pr_info("GDT Initialized\n");
}