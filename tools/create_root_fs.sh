#!/usr/bin/bash
#
# usage: create_root_fs.sh <target_image> <blank_img> <kernel.img> [user programs ...]
#
set -e

#
TARGET_IMAGE="$1"
shift
BLANK_IMAGE="$1"
shift
KERNEL_IMG="$1"
shift

echo "initialize target image $TARGET_IMAGE from blank $BLANK_IMAGE"
cp $BLANK_IMAGE $TARGET_IMAGE


function rootfs_mkdir()
{
    echo "create directory $1"
    mmd -i "$TARGET_IMAGE" ::$1
}

function rootfs_copy()
{
    echo "copy $1 to $2"
    mcopy -i "$TARGET_IMAGE" "$1" ::$2
}

echo "create rootfs layout:"
rootfs_mkdir /bin
rootfs_mkdir /dev
rootfs_mkdir /data

echo "create copy files:"
for f in "$@"; do
    rootfs_copy "$f" /bin/$(basename "$f" | sed -e 's/\.elf//')
done


rootfs_copy "../resources/raspi_zero_files/start.elf" /start.elf
rootfs_copy "../resources/raspi_zero_files/bootcode.bin" /bootcode.bin
rootfs_copy "../resources/raspi_zero_files/config.txt" /config.txt
rootfs_copy "../resources/raspi_zero_files/fixup.dat" /fixup.dat
rootfs_copy "$KERNEL_IMG" /kernel.img
