#include <stddef.h>
#include <stdint-gcc.h>
#include "printk.h"
#include "multiboot.h"
#include "mmu.h"

#define ADD_SIZE 64
#define PAGE(x) (x & ~(PAGE_SIZE-1))


// Keeps track of the current memory region in use
static int curr_region = 0;
static struct mem_regions {
    int num_regions;
    struct region {
        void *addr;
        uint64_t pf_avail; 
    } regions[16];
} mem;

// List of reserved page frames addresses
static struct reserved_pages{
    int num_reserved;
    intptr_t pages[256];
    intptr_t *front;
    intptr_t *end;
} reserved;

static struct pool {
    void *head;
    int num_free;
} pf_pool;

void parse_mmap(void) 
{
    struct memory_map *mmap = get_mmap();
    int num_entries = (mmap->header.size - 16)/mmap->entry_size;
    struct mmap_entry *entries = mmap->entries;

    int num_pages = 0;
    
    // Begin going over mmap entries
    for (int i = 0; i < num_entries; i++) {
        if (entries[i].type == 1) {
            struct region *region = &mem.regions[mem.num_regions++];
            
            region->addr = (void*) entries[i].base_addr;
            region->pf_avail = entries[i].length/PAGE_SIZE;
            num_pages += entries[i].length/PAGE_SIZE;
        }
    }

    pr_info("Total # of Page Frames: %d\n", num_pages);
}

void parse_elf(void)
{
    struct elf_symbols *elf = get_elf();
    struct elf_entry *entries = elf->entries;

    // Begin going over ELF section headers
    for (uint32_t i = 0; i < elf->num_sections; i++) {
        if (entries[i].type != 0) {
            intptr_t page_start = (intptr_t) PAGE(entries[i].mem_addr);
            intptr_t page_end = (intptr_t) PAGE((entries[i].mem_addr + entries[i].size));
            
            // Only add unique reserved pages
            if (page_start == page_end && *(reserved.end-1) != page_start) {
                *reserved.end++ = page_start;
                reserved.num_reserved++;
            }
            else if (page_start != page_end) {
                // Check if first page is already added
                // If so, start from the next page
                if (*(reserved.end-1) == page_start)
                    page_start += PAGE_SIZE;
                
                for (; page_start != page_end; page_start += PAGE_SIZE) {
                    *reserved.end++ = page_start;
                    reserved.num_reserved++;
                }   
            }
        }
        
    }
}

int add_to_pool(void)
{
    // Check if there are available memory regions to 
    // with new page frames
    if (mem.num_regions ==  0)
        return -1;
    
    intptr_t *pf = (intptr_t*) mem.regions[curr_region].addr;

    // Add pages to free list
    while (pf_pool.num_free < ADD_SIZE) {
        
        // Check if page frame is reserved
        if (pf != (intptr_t*) *reserved.front) {
            // Set first 8 bytes to old head of pf_pool
            *pf = (intptr_t) pf_pool.head;
            pf_pool.head = pf;
            pf_pool.num_free++;
        }
        else
            reserved.front++;

        pf += PAGE_SIZE/sizeof(intptr_t);
        mem.regions[curr_region].pf_avail--;

        // Switch to next memory region if there are no more
        // page frames in the current memory region
        if (mem.regions[curr_region].pf_avail == 0) {
            curr_region++;
            mem.num_regions--;
            pf = (intptr_t*) mem.regions[curr_region].addr;
        }

        // Check if all memory regions are exhausted
        if (mem.num_regions == 0) {
            pr_info("All memory regions exhausted!\n");
            return pf_pool.num_free;
        }
    }

    mem.regions[curr_region].addr = (void*) pf;
    return pf_pool.num_free;
}

void *MMU_pf_alloc(void)
{
    // Check if there are available page frames in pool
    if (pf_pool.num_free == 0) {
        if (add_to_pool() < 0) {
            pr_err("Unable to allocate more page frames\n");
            return NULL;
        }
    }

    void *addr = pf_pool.head;
    pf_pool.head = (void*) (*(intptr_t*) pf_pool.head);
    pf_pool.num_free--;
    
    return addr;
}

void MMU_pf_free(void *pf)
{
    intptr_t *page = (intptr_t*) pf;
    *page = (intptr_t) pf_pool.head;
    pf_pool.head = pf;
    pf_pool.num_free++;
}

int MMU_num_reserved(void)
{
    return reserved.num_reserved;
}

void MMU_init(void) 
{
    pr_info("Initializing Memory Management\n");
    
    // Initialize static structures
    mem.num_regions = 0;
    reserved.front = &reserved.pages[0];
    reserved.end = &reserved.pages[0];
    reserved.num_reserved = 0;
    pf_pool.head = NULL;
    pf_pool.num_free = 0;

    // Set page frame 0x0 (NULL) as reserved
    *reserved.end++ = 0x0;
    reserved.num_reserved = 1;

    parse_mmap();
    parse_elf();
    add_to_pool();

    pr_info("Memory Management Initialized\n");
}