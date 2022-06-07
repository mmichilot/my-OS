#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_

extern void PT_init(void);
extern void *MMU_alloc_page();
extern void *MMU_alloc_pages(int num);
extern void MMU_free_page(void* page);
extern void MMU_free_pages(void* page, int num);

#endif