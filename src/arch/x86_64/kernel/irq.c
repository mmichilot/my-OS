#include <stddef.h>
#include <stdint-gcc.h>
#include "irq.h"
#include "exception.h"
#include "idt.h"
#include "printk.h"
#include "drivers/i8259.h"

static struct {
    void *arg;
    irq_handler_t handler;
} irq_table[NUM_IRQS];


void irq_handler(int irq, int error)
{
    if (irq < 0 || irq > 255) {
        pr_err("Invalid IRQ: %d\n", irq);
        asm volatile ("hlt");
    }

    if (irq_table[irq].handler == NULL) {
        pr_err("Unhandled Interrupt: %d\n", irq);
        asm volatile ("hlt");
    }

    irq_table[irq].handler(error, irq_table[irq].arg);

    // Send EOI to PIC
    if (irq > 31 && irq < 48)
        PIC_end_of_interrupt(irq-32);
    
}

void IRQ_set_handler(int irq, irq_handler_t handler, void* arg)
{
    bool enable_ints = false;
    if (are_interrupts_enabled()) {
        enable_ints = true;
        cli();
    }

    if (irq < 0 || irq > 255) {
        pr_err("Invalid IRQ: %d\n", irq);
        return;
    }

    // Configure PIC masking
    if (irq > 31 && irq < 48)
        PIC_clear_mask(irq-32);

    irq_table[irq].handler = handler;
    irq_table[irq].arg = arg;

    if (enable_ints)
        sti();
}

void IRQ_init(void)
{
    cli();
    pr_info("Intializing IRQs\n");

    IDT_init();
    PIC_init();
    setup_handlers();

    pr_info("IRQs Initialized\n");
    sti();
}
