#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#define DIVIDE_BY_ZERO 0
#define GEN_PROTECT    13
#define PAGE_FAULT     14

extern void setup_handlers(void);

#endif