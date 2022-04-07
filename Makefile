ARCH ?= x86_64
KERNEL := build/kernel-$(ARCH).bin
IMG := build/os-$(ARCH).img

SRC_DIR := src/arch/$(ARCH)

LD_SCRIPT := $(SRC_DIR)/linker.ld
GRUB_CFG := $(SRC_DIR)/grub.cfg
ASM_SRC := $(wildcard $(SRC_DIR)/*.asm)
ASM_OBJ := $(patsubst $(SRC_DIR)/%.asm, build/arch/$(ARCH)/%.o, $(ASM_SRC))

QEMU_OPTS := -s -drive format=raw,file=$(IMG) -serial stdio

.PHONY: all clean run img

all: $(KERNEL)

clean:
	@rm -r build

run: $(IMG)
	@qemu-system-x86_64 $(QEMU_OPTS)

img: $(IMG)

$(IMG): $(KERNEL) $(GRUB_CFG)
# Create boot directory and copy files
	@mkdir -p build/imgfiles/boot/grub
	@cp $(KERNEL) build/imgfiles/boot/kernel.bin
	@cp $(GRUB_CFG) build/imgfiles/boot/grub

# Run script to create image (have to run as root!)
	@echo 'Creating image (must run as root!)'
	@sudo ./create_img.sh

$(KERNEL): $(ASM_OBJ) $(LD_SCRIPT)
	@ld -n -T $(LD_SCRIPT) -o $(KERNEL) $(ASM_OBJ)

# Compile assembly files
build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -f elf64 $< -o $@