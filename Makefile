# Parameters
ARCH ?= x86_64
KERNEL := build/kernel-$(ARCH).bin
IMG := build/os-$(ARCH).img

# Required tools (be sure to add tools to $PATH)
CC := $(ARCH)-linux-gcc
LD := $(ARCH)-linux-ld

# Directories
SRC_DIR := src/arch/$(ARCH)
INCLUDE_DIR := include/arch/$(ARCH)

# Source files
LD_SCRIPT := $(SRC_DIR)/linker.ld
GRUB_CFG := $(SRC_DIR)/grub.cfg
ASM_SRC := $(wildcard $(SRC_DIR)/*.asm)
ASM_OBJ := $(patsubst $(SRC_DIR)/%.asm, build/arch/$(ARCH)/%.o, $(ASM_SRC))
C_SRC := $(wildcard $(SRC_DIR)/*.c)
C_OBJ := $(patsubst $(SRC_DIR)/%.c, build/arch/$(ARCH)/%.o, $(C_SRC))

# Tool options
CFLAGS := -g -c -I$(INCLUDE_DIR) -std=gnu99 -ffreestanding -Wall -Wextra
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG
endif

LDFLAGS := -n -T $(LD_SCRIPT) -o $(KERNEL)
QEMU_OPTS := -s -drive format=raw,file=$(IMG) -serial stdio

.PHONY: all clean run img

all: $(KERNEL)

# Compile assembly files
build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -f elf64 $< -o $@

# Compile C files
build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.c
	$(CC) $(CFLAGS) $< -o $@

# Link all object files
$(KERNEL): $(ASM_OBJ) $(C_OBJ) $(LD_SCRIPT)
	$(LD) $(LDFLAGS) $(ASM_OBJ) $(C_OBJ)

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
