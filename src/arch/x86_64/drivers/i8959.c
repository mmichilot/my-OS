#include "port-io.h"
#include "printk.h"
#include "drivers/i8259.h"

#define PIC1            0x20    /* IO base address for main PIC */
#define PIC2            0xA0    /* IO base address for secondary PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)
#define PIC1_OFFSET     0x20
#define PIC2_OFFSET     0x28
#define PIC1_IRQ_CAS    0x04
#define PIC2_IRQ_ID     0x02

#define ICW1        0x10
#define ICW1_ICW4   (1 << 0)
#define ICW1_SNGL   (1 << 1)
#define ICW1_ADI    (1 << 2)
#define ICW1_LTIM   (1 << 3)

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */

#define PIC_EOI		0x20		/* End-of-interrupt command code */


void PIC_init(void)
{
    pr_info("Initializing i8259 PIC\n");


    // Begin PIC initialization
    outb(PIC1_COMMAND, ICW1);
    outb(PIC2_COMMAND, ICW1);

    // Set PIC offsets
    outb(PIC1_DATA, PIC1_OFFSET);
    outb(PIC2_DATA, PIC2_OFFSET);

    outb(PIC1_DATA, PIC1_IRQ_CAS);
    outb(PIC2_DATA, PIC2_IRQ_ID);

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    // Mask all interrupts
    outb(PIC1_DATA, 0xFB);
    outb(PIC2_DATA, 0xFF);

    pr_info("i8259 PIC Initialized\n");
}
 
void PIC_end_of_interrupt(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
 
	outb(PIC1_COMMAND,PIC_EOI);
}

void PIC_set_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);     
}

void PIC_clear_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);   
}


uint16_t PIC_get_mask(void) {
    uint16_t mask;

    mask = inb(PIC1_DATA);
    mask |= (inb(PIC2_DATA) << 8);

    return mask;
}