#include <stdint-gcc.h>
#include "gdt.h"
#include "printk.h"

extern void* isr_stacks[];

struct TSS_entry {
    uint16_t limit1;    // TSS Limit 1
    uint16_t base1;     // TSS Address 1 
    uint8_t  base2;     // TSS address 2
    uint8_t  access;    
    uint8_t  limit2:4;  // TSS Limit 2
    uint8_t  flags:4;
    uint8_t  base3;     // TSS address 3
    uint32_t base4;     // TSS address 4
    uint32_t :32;       // Reserved
} __attribute__((packed));

static struct {
    uint32_t :32;       // Segment Limit 1 and Base Address 1 (ignored in long mode)
    uint8_t  :8;        // Base address 2 (ignored in long mode)
    uint8_t  access;    
    uint8_t  :4;        // Segment Limit 2 (ignored in long mode)
    uint8_t  flags:4;
    uint8_t  :8;        // Base address 3 (ignored in long mode)
} __attribute__((packed)) GDT[6];

static struct {
    uint32_t :32;       // Reserved
    uint64_t rsp[3];    // Stack Pointers
    uint64_t ist[8];    // Interrupt Stack Table
    uint64_t :64;       // Reserved
    uint16_t :16;       // Reserved
    uint16_t iopb;      // IO Map Base Address
} __attribute__((packed)) TSS;

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

static inline void ltr(uint16_t offset) 
{
    asm volatile ( "ltr %0" : : "r"(offset) );
}

void GDT_init(void)
{
    pr_info("Initializing GDT\n");

    // Configure GDT Entries
    GDT[KERNEL_CS].access = (EXEC | TYPE_CS | DPL_KERNEL | PRESENT);
    GDT[KERNEL_CS].flags = LONG_MODE;

    GDT[USER_CS].access = (EXEC | TYPE_CS | DPL_USER | PRESENT);
    GDT[USER_CS].flags = LONG_MODE;

    GDT[USER_DS].access = (TYPE_DS | PRESENT);
    
    // Configure TSS
    TSS.ist[GP_IST] = (uint64_t) isr_stacks[GP_IST];
    TSS.ist[DF_IST] = (uint64_t) isr_stacks[DF_IST];
    TSS.ist[PF_IST] = (uint64_t) isr_stacks[PF_IST];
    TSS.iopb = (uint16_t) sizeof(TSS);

    // Add TSS descriptor to GDT
    struct TSS_entry* TSS_seg = (struct TSS_entry*) &GDT[TSS_DESC]; 
    uint64_t TSS_addr = (uint64_t) &TSS;
    uint32_t TSS_size = (uint32_t) sizeof(TSS);
    TSS_seg->base1 = TSS_addr & 0xFFFF;
    TSS_seg->base2 = (TSS_addr >> 16) & 0xFF;
    TSS_seg->base3 = (TSS_addr >> 24) & 0xFF;
    TSS_seg->base4 = (TSS_addr >> 32) & 0xFFFFFFFF;
    TSS_seg->limit1 = TSS_size & 0xFFFF;
    TSS_seg->limit2 = (TSS_size >> 16) & 0xF;
    TSS_seg->access = (TSS_64 | TYPE_TSS | DPL_KERNEL | PRESENT);
    TSS_seg->flags = LONG_MODE;

    lgdt((void*)GDT, sizeof(GDT));
    ltr(TSS_OFFSET);
    
    pr_info("GDT Initialized\n");
}