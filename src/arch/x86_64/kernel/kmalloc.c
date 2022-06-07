// Using Fixed-size block pools (following Dr. Bellardo's implementation)
#include <stddef.h>
#include <stdint-gcc.h>
#include "printk.h"
#include "page_table.h"

#define PAGE_SIZE 4096
#define NUM_POOLS 6
#define BASE_SIZE 64

struct free_list {
    struct free_list *next;
};

struct kmalloc_pool {
    int max_size;
    int avail;
    struct free_list *head;
};

struct kmalloc_extra {
    struct kmalloc_pool *pool;
    size_t size;
};

// Pools
static struct kmalloc_pool pools[NUM_POOLS];

void add_blocks(struct kmalloc_pool *pool)
{
    // Get virtual page frame
    pool->head = (struct free_list*) MMU_alloc_page();

    // Split page frame into blocks
    struct free_list *addr = pool->head;
    for (int i = 0; i < PAGE_SIZE/pool->max_size; i++) {
        addr->next = (struct free_list*) (((char*) addr) + pool->max_size);
        addr = addr->next;
    }

    printk("Page Contents\n");
    for (int i = 0; i < 512; i++) {
        printk("[%d] 0x%lx\n", i, *(((uint64_t*)pool->head) + i));
    }

}

void kmalloc_init(void) {
    pr_info("Initializing Heap Allocator (kmalloc)\n");

    // Setup pools
    for (int i = 0; i < NUM_POOLS; i++) {
        pools[i].max_size = BASE_SIZE << i;
        printk("Setting up %d B pool\n", pools[i].max_size);
        pools[i].avail = PAGE_SIZE/pools[i].max_size;
        add_blocks(&pools[i]);
    }
}