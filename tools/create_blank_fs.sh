#!/usr/bin/bash
#
# usage: create_blank_fs.sh <blank_img>
#
TARGET_IMAGE="$1"
echo "create blank fat32 SD image $TARGET_IMAGE"

truncate -s 4G $TARGET_IMAGE
mkfs.fat -F 32 -I -s 8 -S 512 $TARGET_IMAGE

