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
    printk("Adding more blocks into %d B pool\n", pool->max_size);
    // Get virtual page frame
    struct free_list *addr = (struct free_list*) MMU_alloc_page();
    struct free_list *block = addr;

    for (int i = 0; i < (PAGE_SIZE/pool->max_size); i++) {
        block->next = (struct free_list*) (((char*) block) + pool->max_size);
        block = block->next;
    }

    if (pool->head == NULL)
        pool->head = addr;
    else
        pool->head->next = addr;
    
    pool->avail += PAGE_SIZE/pool->max_size;
}

void kfree(void *addr)
{
    struct kmalloc_extra *metadata = (struct kmalloc_extra*) ((char*) addr - sizeof(struct kmalloc_extra));
    
    // Determine if block came from pool
    if (metadata->pool != NULL) {
        // Add block back to pool
        struct kmalloc_pool *pool = metadata->pool;
        struct free_list* header = (struct free_list*) metadata;

        printk("Putting block into %d B pool\n", pool->max_size);

        header->next = pool->head;

        pool->head = header;
        pool->avail++;

        printk("# of available blocks: %d\n", pool->avail);

        return;
    }

    // Free virtual pages if block didn't come from pool
    size_t size = metadata->size;
    int num_pages = (size % PAGE_SIZE) ? ((size/PAGE_SIZE) + 1) : (size/PAGE_SIZE);
    printk("Freeing %d virtual page(s)\n", num_pages);
    MMU_free_pages((void*) metadata, num_pages);
}

void *kmalloc(size_t size)
{
    // Iterate though each pool to detemine which one can
    // accomodate allocation
    size += sizeof(struct kmalloc_extra);
    for (unsigned int i = 0; i < NUM_POOLS; i++) {
        if (pools[i].max_size >= (int) size) {
            struct kmalloc_pool *pool = &pools[i];
            struct kmalloc_extra *block = (struct kmalloc_extra*) pool->head;
            
            printk("Grabbing block from %d B pool\n", pool->max_size);
            printk("# of available blocks: %d\n", pool->avail);
            
            // Add more blocks if necessary
            if (pool->avail == 1)
                add_blocks(pool);
            
            pool->head = pool->head->next;
            pool->avail--;                

            block->pool = pool;
            block->size = size;

            return (void*) ((char*) block + sizeof(struct kmalloc_extra));
        }
    }

    // If no pools can accomodate allocation, allocate contiguous virtual frames
    int num_pages = (size % PAGE_SIZE) ? ((size/PAGE_SIZE) + 1) : (size/PAGE_SIZE);
    printk("No suitable pools found, allocating %d virtual page(s)\n", num_pages);
    struct kmalloc_extra *block = (struct kmalloc_extra*) MMU_alloc_pages(num_pages);
    block->pool = NULL;
    block->size = size;

    return (void*) ((char*) block + sizeof(struct kmalloc_extra));
}

void kmalloc_init(void) {
    pr_info("Initializing Heap Allocator (kmalloc)\n");

    // Setup pools
    // 64 B, 128 B, 256 B, 512 B, 1024 B, 2048 B
    for (int i = 0; i < NUM_POOLS; i++) {
        pools[i].max_size = BASE_SIZE << i;
        pools[i].avail = 0;
        add_blocks(&pools[i]);
    }
}