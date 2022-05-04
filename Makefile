# Parameters
ARCH ?= x86_64
KERNEL := build/kernel-$(ARCH).bin
IMG := build/os-$(ARCH).img

# Required tools (be sure to add tools to $PATH)
CC := $(ARCH)-linux-gcc
LD := $(ARCH)-linux-ld

# Directories
SRC_DIR := src/arch/$(ARCH)
BOOT_DIR := $(SRC_DIR)/boot
KERNEL_DIR := $(SRC_DIR)/kernel
DRIVER_DIR := $(SRC_DIR)/drivers
INCLUDE_DIR := include/arch/$(ARCH)

# Source files
LD_SCRIPT := $(BOOT_DIR)/linker.ld
GRUB_CFG := $(BOOT_DIR)/grub.cfg
ASM_SRC := $(wildcard $(BOOT_DIR)/*.asm)
ASM_OBJ := $(patsubst $(BOOT_DIR)/%.asm, build/arch/$(ARCH)/%.o, $(ASM_SRC))
KERNEL_SRC := $(wildcard $(KERNEL_DIR)/*.c) 
KERNEL_OBJ := $(patsubst $(KERNEL_DIR)/%.c, build/arch/$(ARCH)/%.o, $(KERNEL_SRC))
DRIVER_SRC := $(wildcard $(DRIVER_DIR)/*.c)
DRIVER_OBJ := $(patsubst $(DRIVER_DIR)/%.c, build/arch/$(ARCH)/%.o, $(DRIVER_SRC))

# Tool options
CFLAGS := -g -c -I$(INCLUDE_DIR) -std=gnu99 -ffreestanding -Wall -Wextra -Werror -mno-red-zone

LDFLAGS := -n -T $(LD_SCRIPT) -o $(KERNEL)
QEMU_OPTS := -s -drive format=raw,file=$(IMG) -serial stdio

.PHONY: all clean run img

all: $(KERNEL)

# Compile assembly files
build/arch/$(ARCH)/%.o: $(BOOT_DIR)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -f elf64 $< -o $@

# Compile C files
build/arch/$(ARCH)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

build/arch/$(ARCH)/%.o: $(DRIVER_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

# Link all object files
$(KERNEL): $(ASM_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) $(ASM_OBJ) $(KERNEL_OBJ) $(DRIVER_OBJ)

img: $(IMG)

$(IMG): $(KERNEL) $(GRUB_CFG)
# Create boot directory and copy files
	@mkdir -p build/imgfiles/boot/grub
	@cp $(KERNEL) build/imgfiles/boot/kernel.bin
	@cp $(GRUB_CFG) build/imgfiles/boot/grub
# Run script to create image (have to run as root!)
	@echo 'Creating image (must run as root!)'
	@sudo ./create_img.sh

run: $(IMG)
	@qemu-system-x86_64 $(QEMU_OPTS)

clean:
	@rm -r build
