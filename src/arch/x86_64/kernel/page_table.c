#include <stdint-gcc.h>
#include <stddef.h>
#include "string.h"
#include "printk.h"
#include "mmu.h"

#define PT_OFFSET 12
#define PD_OFFSET 21
#define PDP_OFFSET 30
#define PML4_OFFSET 39
#define INDEX(offset, x) ((x >> offset) & 0x1FF)

struct entry {
    uint64_t base:52;
    uint16_t :12;
} __attribute__((packed));

static struct entry pml4_table[512] __attribute__((aligned(PAGE_SIZE)));

static inline uint64_t read_cr3()
{
    uint64_t val = 0;
    asm volatile( "mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void reload_cr3(void *base) 
{
    intptr_t cr3 = (intptr_t) base;
    asm volatile ( "mov %0, %%cr3" : : "r"(cr3) );
}

static inline void __native_flush_tlb_single(unsigned long addr) {
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

/* Walks the page table to get the page table entry
 * for a specified virtual address
 */
void *get_pte(void *pml4_table, void *virt_addr)
{
    // Get table indices
    int pml4_index = INDEX(PML4_OFFSET, (intptr_t) virt_addr);
    int pdp_index  = INDEX(PDP_OFFSET,  (intptr_t) virt_addr);
    int pd_index   = INDEX(PD_OFFSET,   (intptr_t) virt_addr);
    int pt_index   = INDEX(PT_OFFSET,   (intptr_t) virt_addr);

    struct entry *pml4_entry = (struct entry*) pml4_table + pml4_index;
    // Check if PML4 entry exists
    if (!pml4_entry->base & 0x1) {
        pml4_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pml4_entry->base |= 0x3;
    }

    
    struct entry *pdp_table = (struct entry*) (((intptr_t) pml4_entry->base & ~0xFFF));
    struct entry *pdp_entry = pdp_table + pdp_index;
    // Check if PDP entry exists
    if (!pdp_entry->base & 0x1) {
        pdp_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pdp_entry->base |= 0x3;
    }

    
    struct entry *pd_table = (struct entry*) (((intptr_t) pdp_entry->base & ~0xFFF));
    struct entry *pd_entry = pd_table + pd_index;
    // Check if PD entry exists
    if (!pd_entry->base & 0x1) {
        pd_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pd_entry->base |= 0x3;
    }

    struct entry *page_table = (struct entry*) (((intptr_t) pd_entry->base & ~0xFFF));

    return (void*) (page_table + pt_index);

}

void PT_init(void) 
{
    pr_info("Initializing Page Table\n");

    // Set up identity paging for first 2MB
    intptr_t addr = PAGE_SIZE;
    for (int i = 1; i < 512; i++) {
        struct entry *pte = (struct entry *) get_pte((void*) pml4_table, (void*) addr);
        pte->base = addr | 0x3;
        addr += PAGE_SIZE;
    }

    // Reload CR3
    reload_cr3((void*) pml4_table);

    pr_info("Page Table Initialized\n");
}