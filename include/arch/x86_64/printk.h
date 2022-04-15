#ifndef _PRINTK_H_
#define _PRINTK_H_

#include <stdarg.h>

extern int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif