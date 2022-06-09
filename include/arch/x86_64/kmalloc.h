#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#include <stddef.h>

extern void kmalloc_init(void);
extern void *kmalloc(size_t size);
extern void kfree(void *addr);

#endif