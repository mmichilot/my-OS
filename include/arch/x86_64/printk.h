#ifndef _PRINTK_H_
#define _PRINTK_H_

#include <stdarg.h>

#define SOH "\001"

#define ERR SOH "1"
#define INFO SOH "2"

#define pr_err(fmt, ...) \
        printk(ERR fmt, ##__VA_ARGS__)

#define pr_info(fmt, ...) \
        printk(INFO fmt, ##__VA_ARGS__)

extern int printk(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif