#!/bin/bash

ARCH='x86_64'
KERNEL="build/kernel-${ARCH}.bin"
IMG="build/os-${ARCH}.img"
MOUNTDIR="$(pwd)/image"

# Create image file
dd if=/dev/zero of=${IMG} bs=512 count=32768
echo 'label: dos' | sfdisk ${IMG}
echo 'start=2048, size=28673, type=b, bootable' | sfdisk ${IMG}

# Setup image
mkdir -p ${MOUNTDIR}
losetup /dev/loop0 ${IMG}
losetup /dev/loop1 ${IMG} -o 1048576
mkdosfs -F 32 -f 2 /dev/loop1

# Install GRUB bootloader
mount /dev/loop1 ${MOUNTDIR}
grub-install --root-directory=${MOUNTDIR} --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0
cp -r build/imgfiles/* ${MOUNTDIR}

# Cleanup
umount ${MOUNTDIR}
losetup -d /dev/loop0
losetup -d /dev/loop1
rmdir ${MOUNTDIR}

# Make image file accessible to user
chown mmichilot:mmichilot ${IMG}