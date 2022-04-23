#define PIC1            0x20    /* IO base address for main PIC */
#define PIC2            0xA0    /* IO base address for secondary PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

void PIC_init(void) {
    
}