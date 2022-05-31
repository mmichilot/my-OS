#include <stdint-gcc.h>
#include <stddef.h>
#include "printk.h"
#include "multiboot.h"

static struct memory_map *mmap = NULL;
static struct elf_symbols *elf = NULL;

void parse_tags(void *tags) 
{
    pr_info("Parsing Multiboot Tags\n");

    struct multiboot_header *header = (struct multiboot_header*) tags;
    printk("Total Size: %d\n", header->size);

    // Begin parsing through tags
    struct tag_header *tag;
    for (tag = tags+8; tag->type != 0; tag = NEXT_TAG(tag)) {
        switch (tag->type) {
            case BOOT_CMDLINE:
                struct boot_cmdline *cmdline =  (struct boot_cmdline*) tag;
                printk("cmdline=%s\n", cmdline->cmdline);
                break;
            case BOOTLOADER_NAME:
                struct bootloader_name *name = (struct bootloader_name*) tag;
                printk("Bootloader: %s\n", name->name);
                break;
            case BASIC_MEM_INFO:
                struct basic_mem_info *basic_mem = (struct basic_mem_info*) tag;
                printk("Basic Memory Info: lower=%u KB, upper=%u KB\n", basic_mem->mem_lower, basic_mem->mem_upper);
                break;
            case MEMORY_MAP:
                mmap = (struct memory_map*) tag;
                printk("Memory Map: entry_size=%u, entry_version=%u\n", mmap->entry_size, mmap->entry_version);
                break;
            case ELF_SYMBOLS:
                elf = (struct elf_symbols*) tag;
                printk("ELF Symbols: num_sections=%u, section_size=%u, string_table_idx=%u\n",
                    elf->num_sections, elf->section_size, elf->string_table);
            default:
                break;
        }
    }

    pr_info("Parsing Multiboot Tags Complete\n");
}

void print_mmap(struct memory_map *mmap)
{
    int num_entries = (mmap->header.size - 16)/mmap->entry_size;
    struct mmap_entry *entries = mmap->entries;

    printk("Memory Map Entries:\n");
    int i;
    for (i = 0; i < num_entries; i++) {
        printk("[%d] base_addr=0x%lx, length=%lu b, type=%u\n", 
            i, entries[i].base_addr, entries[i].length, entries[i].type);
    }
}

void print_elf(struct elf_symbols *elf)
{
    int num_sections = elf->num_sections;
    struct elf_entry *entries = elf->entries;
    char *strtab = (char*) entries[elf->string_table].mem_addr;
    int i;
    for (i = 0; i < num_sections; i++) {
        printk("[%d] %s: type=%d, addr=0x%lx, size=%lu b\n", 
            i, strtab+entries[i].name, entries[i].type, entries[i].mem_addr, entries[i].size);
    }
}

struct memory_map* get_mmap(void)
{
    return mmap;
}

struct elf_symbols* get_elf(void)
{
    return elf;
}