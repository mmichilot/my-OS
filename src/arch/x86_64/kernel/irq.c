#include <stddef.h>
#include <stdint-gcc.h>
#include "irq.h"
#include "printk.h"
#include "i8259.h"

static struct {
    void *arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];

static inline void lidt(void* base, uint16_t size)
{
    struct {
        uint16_t length;
        void* base;
    } __attribute__((packed)) IDTR = {size, base};

    asm volatile ( "lidt %0" : : "m"(IDTR));
}

void irq_handler(int irq, int error)
{
    if (irq < 0 || irq > 255) {
        pr_err("Invalid IRQ: %d\n", irq);
        asm volatile ("hlt");
    }

    if (irq_table[irq].handler == NULL) {
        pr_err("Unhandled Interrupt\n");
        asm volatile ("hlt");
    }

    irq_table[irq].handler(error, irq_table[irq].arg);

}

void IRQ_init(void)
{
    cli();
    PIC_init();
}

void IRQ_set_handler(int irq, irq_handler_t handler, void* arg)
{
    if (irq < 0 || irq > 255) {
        pr_err("Invalid IRQ: %d\n", irq);
        return;
    }

    irq_table[irq].handler = handler;
    irq_table[irq].arg = arg;
}
