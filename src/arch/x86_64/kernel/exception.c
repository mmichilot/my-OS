#include <stdint-gcc.h>
#include <stddef.h>
#include "exception.h"
#include "irq.h"
#include "printk.h"

void divide_by_zero(int error, void *arg)
{
    UNUSED(arg);
    pr_err("Divide by zero fault!\n");
    pr_err("Faulting Instruction Address: 0x%x\n", error);

    pr_err("Halting!");
    asm volatile ("hlt");
}

void general_protection(int error, void *arg)
{
    UNUSED(arg);
    pr_err("General protection fault!\n");
    pr_err("Error code: 0x%x\n", error);

    pr_err("Halting!");
    asm volatile ("hlt");
}

void setup_handlers(void)
{
    IRQ_set_handler(DIVIDE_BY_ZERO, divide_by_zero, NULL);
    IRQ_set_handler(GEN_PROTECT, general_protection, NULL);
}