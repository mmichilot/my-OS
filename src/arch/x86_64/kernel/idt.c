#include <stdint-gcc.h>
#include "irq.h"
#include "gdt.h"
#include "printk.h"

extern void* isr_stub_table[];

#define INT_GATE    0xE
#define TRAP_GATE   0xF

static struct {
    uint16_t    isr1;
    uint16_t    kernel_cs;
    uint8_t     ist;
    uint8_t     attributes;
    uint16_t    isr2;
    uint32_t    isr3;
    uint32_t    :32;
} __attribute__((packed)) IDT[NUM_IRQS];

static inline void lidt(void* base, uint16_t size)
{
    struct {
        uint16_t length;
        void* base;
    } __attribute__((packed)) IDTR;

    IDTR.length = size;
    IDTR.base = base;

    asm volatile ( "lidt %0" : : "m"(IDTR));
}

void IDT_init(void)
{
    pr_info("Initializing IDT\n");

    uintptr_t addr;
    int i;

    for (i = 0; i < NUM_IRQS; i++) {
        addr = (uintptr_t) isr_stub_table[i];
        IDT[i].isr1 = addr & 0xFFFF;
        IDT[i].isr2 = (addr >> 16) & 0xFFFF;
        IDT[i].isr3 = (addr >> 32) & 0xFFFFFFFF;

        IDT[i].kernel_cs = KERNEL_CS_OFFSET;
        IDT[i].ist = 0;
        IDT[i].attributes = (INT_GATE | DPL_KERNEL | PRESENT);
    }

    lidt((void*)IDT, sizeof(IDT));

    pr_info("IDT Initialized\n");
}