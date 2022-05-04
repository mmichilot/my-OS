#ifndef _GDT_H_
#define _GDT_H_

#include <stdint-gcc.h>

/* GDT Segment Indices */
#define NULL_DESC   0
#define KERNEL_CS   1
#define USER_CS     2
#define USER_DS     3
#define TSS         4

/* GDT Segment Offsets */
#define KERNEL_CS_OFFSET    8*KERNEL_CS
#define USER_CS_OFFSET      8*USER_CS
#define USER_DS_OFFSET      8*USER_DS
#define TSS_OFFSET          8*TSS

/* Segment Descriptor Access */
#define ACCESSED    (1 << 0)
#define RW          (1 << 1)
#define DIR         (1 << 2)
#define CONFORM     DIR
#define EXEC        (1 << 3)
#define TYPE(x)     (x << 4)   
#define DPL(x)      (x << 5)
#define PRESENT     (1 << 7)

#define DPL_KERNEL  DPL(0)
#define DPL_USER    DPL(3)
#define TYPE_CS     TYPE(1)
#define TYPE_DS     TYPE(1)
#define TYPE_TSS    TYPE(0)

/* Segment Descriptor Flags */
#define LONG_MODE   (1 << 1)
#define SIZE        (1 << 2)
#define PAGE_GRAN   (1 << 3)

extern void GDT_init(void);

#endif