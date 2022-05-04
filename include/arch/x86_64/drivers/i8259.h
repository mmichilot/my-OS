#ifndef _I8259_H_
#define _I8259_H_

#include <stdint-gcc.h>

extern void PIC_init(void);
extern void PIC_set_mask(uint8_t irq);
extern void PIC_clear_mask(uint8_t irq);
extern uint16_t PIC_get_mask(void);
extern void PIC_end_of_interrupt(uint8_t irq);

#endif