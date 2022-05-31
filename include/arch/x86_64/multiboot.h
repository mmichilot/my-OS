#ifndef _MULTIBOOT_H_
#define _MULTIBOOT_H_

#include <stdint-gcc.h>

enum TAG_TYPES {
    BOOT_CMDLINE    = 1,
    BOOTLOADER_NAME = 2,
    BASIC_MEM_INFO  = 4,
    BIOS_BOOT_DEV   = 5,
    MEMORY_MAP      = 6,
    FRAMEBUF_INFO   = 8,
    ELF_SYMBOLS     = 9,
    APM_TABLE       = 10,
    ACPI_OLD_RSDP   = 14,
    IMAGE_LD_ADDR   = 21
};

enum MEMORY_TYPES {
    MEM_AVAILABLE = 1,
    MEM_RESERVED  = 2
};


struct multiboot_header {
    uint32_t size;
    uint32_t :32;
} __attribute__((packed));

struct tag_header {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct boot_cmdline {
    struct tag_header header;
    char cmdline[];
} __attribute__((packed));

struct bootloader_name {
    struct tag_header header;
    char name[];
} __attribute__((packed));

struct basic_mem_info {
    struct tag_header header;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed));

struct mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t :32;
} __attribute__((packed));

struct memory_map {
    struct tag_header header;
    uint32_t entry_size;
    uint32_t entry_version;
    struct mmap_entry entries[];
} __attribute__((packed));

struct elf_entry {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t mem_addr;
    uint64_t disk_offset;
    uint64_t size;
    uint32_t table_index;
    uint32_t extra;
    uint64_t addr_align;
    uint64_t :64;
} __attribute__((packed));

struct elf_symbols {
    struct tag_header header;
    uint32_t num_sections;
    uint32_t section_size;
    uint32_t string_table;
    struct elf_entry entries[];
} __attribute__((packed));

#define NEXT_TAG(tag) ((struct tag_header*) ((uint8_t*) tag + ((tag->size + 7) & ~7)))

extern void parse_tags(void *tags);
void print_mmap(struct memory_map *mmap);
void print_elf(struct elf_symbols *elf);
struct memory_map* get_mmap(void);
struct elf_symbols* get_elf(void);

#endif