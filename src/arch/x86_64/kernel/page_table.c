#include <stdint-gcc.h>
#include <stddef.h>
#include "string.h"
#include "printk.h"
#include "mmu.h"
#include "irq.h"

// Address space offsets
#define KERNEL_STACKS 0x10000000000
#define KERNEL_HEAP   0xF0000000000

#define PT_OFFSET 12
#define PD_OFFSET 21
#define PDP_OFFSET 30
#define PML4_OFFSET 39
#define INDEX(offset, x) ((x >> offset) & 0x1FF)

// Page Table entry bits
#define BIT(x) (1 << x)
#define PRESENT   BIT(0)
#define WRITEABLE BIT(1)
#define USER      BIT(2)
#define DEMAND    BIT(9)

// Page Fault error bits
#define ERR_PRESENT BIT(0)

struct entry {
    uint64_t base:52;
    uint16_t :12;
} __attribute__((packed));

static struct entry pml4_table[512] __attribute__((aligned(PAGE_SIZE)));
static void* brk = (void*) KERNEL_HEAP;


static inline uint64_t read_cr2()
{
    uint64_t val = 0;
    asm volatile( "mov %%cr2, %0" : "=r"(val));
    return val;
}

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
    bool enable_ints = false;
    if (are_interrupts_enabled()) {
        enable_ints = true;
        cli();
    }

    // Get table indices
    int pml4_index = INDEX(PML4_OFFSET, (intptr_t) virt_addr);
    int pdp_index  = INDEX(PDP_OFFSET,  (intptr_t) virt_addr);
    int pd_index   = INDEX(PD_OFFSET,   (intptr_t) virt_addr);
    int pt_index   = INDEX(PT_OFFSET,   (intptr_t) virt_addr);

    struct entry *pml4_entry = (struct entry*) pml4_table + pml4_index;
    // Check if PML4 entry exists
    if (!(pml4_entry->base & PRESENT)) {
        pml4_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pml4_entry->base |= (PRESENT | WRITEABLE);
    }
    
    struct entry *pdp_table = (struct entry*) (((intptr_t) pml4_entry->base & ~0xFFF));
    struct entry *pdp_entry = pdp_table + pdp_index;
    // Check if PDP entry exists
    if (!(pdp_entry->base & PRESENT)) {
        pdp_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pdp_entry->base |= (PRESENT | WRITEABLE);
    }
    
    struct entry *pd_table = (struct entry*) (((intptr_t) pdp_entry->base & ~0xFFF));
    struct entry *pd_entry = pd_table + pd_index;
    // Check if PD entry exists
    if (!(pd_entry->base & PRESENT)) {
        pd_entry->base = (intptr_t) memset(MMU_pf_alloc(), 0, PAGE_SIZE);
        pd_entry->base |= (PRESENT | WRITEABLE);
    }

    struct entry *page_table = (struct entry*) (((intptr_t) pd_entry->base & ~0xFFF));
    
    if (enable_ints)
        sti();
    
    return (void*) (page_table + pt_index);

}

void *MMU_alloc_page()
{
    // Save brk address
    void *addr = brk;
    
    // Allocate page
    struct entry *pte = (struct entry*) get_pte((void*) pml4_table, brk);
    pte->base |= DEMAND;

    // Have brk point to next unused page
    brk += PAGE_SIZE;
    
    return addr;
}

void *MMU_alloc_pages(int num)
{
    // Save address of brk
    void *addr = brk;

    // Allocate pages
    for (int i = 0; i < num; i++) {
        struct entry *pte = (struct entry*) get_pte((void*) pml4_table, brk);
        pte->base |= DEMAND;
        brk += PAGE_SIZE;
    }

    return addr;
}

void MMU_free_page(void *page)
{
    // Get page table entry
    struct entry *pte = (struct entry*) get_pte((void*) pml4_table, page);
    MMU_pf_free((void*) ((intptr_t) pte->base & ~0xFFF));
    pte->base = 0;

    // Flush TLB entry
    __native_flush_tlb_single((intptr_t) page);
}

void MMU_free_pages(void *page, int num)
{
    void* addr = page;
    
    // Deallocate pages
    for (int i = 0; i < num; i++) {
        struct entry *pte = (struct entry*) get_pte((void*) pml4_table, addr);
        MMU_pf_free( (void*) ((intptr_t) pte->base & ~0xFFF));
        pte->base = 0;

        // Flush TLB entry
        __native_flush_tlb_single((intptr_t) page);

        // Go to next page
        addr += PAGE_SIZE;
    }
}

void page_fault_handler(int error, void* arg)
{
    UNUSED(arg);

    // Get page fault address
    uint64_t virt_addr = read_cr2();

    // Check demand bit and error code
    struct entry *pte = (struct entry*) get_pte((void*) pml4_table, (void*) (virt_addr & ~0xFFF));
    if ((pte->base & DEMAND) && !(error & ERR_PRESENT)) {
        pr_info("Allocating page: 0x%lx\n", (virt_addr & ~0xFFF));
        // Set demand bit to 0
        pte->base &= ~DEMAND;
        
        // Allocate page
        pte->base = (intptr_t) MMU_pf_alloc();
        pte->base |= (PRESENT | WRITEABLE);

        return;
    }

    pr_err("Unhandled Page Fault!\n");
    pr_err("Address that caused fault: %p\n", (void*) virt_addr);
    pr_err("Page table address: 0x%lx\n", read_cr3());
    pr_err("Fault code: 0x%x\n", error);

    asm volatile ("hlt");
}

void PT_init(void) 
{
    pr_info("Initializing Page Table\n");

    // Set up identity paging for first 2MB
    for (intptr_t addr = PAGE_SIZE; addr < 512*PAGE_SIZE; addr += PAGE_SIZE) {
        struct entry *pte = (struct entry *) get_pte((void*) pml4_table, (void*) addr);
        pte->base = (addr & ~0xFFF) | (PRESENT | WRITEABLE);
    }

    // Reload CR3
    reload_cr3((void*) pml4_table);

    IRQ_set_handler(14, page_fault_handler, NULL);
    pr_info("Registered Page Fault Handler @ IRQ %d\n", 14);

    pr_info("Page Table Initialized\n");
}