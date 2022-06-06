#ifndef _MMU_H_
#define _MMU_H_

#define PAGE_SIZE 4096

extern void MMU_init(void);
extern void *MMU_pf_alloc(void);
extern void MMU_pf_free(void *pf);
extern void test_MMU(void);
extern void stress_MMU(void);

#endif