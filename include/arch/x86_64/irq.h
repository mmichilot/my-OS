#ifndef _IRQ_H_
#define _IRQ_H_

#include <stdint-gcc.h>
#include <stdbool.h>

#define NUM_IRQS 256
#define UNUSED(x) (void) x

typedef void (*irq_handler_t)(int, void*);

extern void IRQ_init(void);
extern void IRQ_set_handler(int irq, irq_handler_t handler, void* arg);

static inline bool are_interrupts_enabled()
{
    unsigned long flags;
    
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    
    return flags & (1 << 9);
}

static inline void cli() { asm volatile ( "cli" ); }
static inline void sti() { asm volatile ( "sti" ); }

#endif