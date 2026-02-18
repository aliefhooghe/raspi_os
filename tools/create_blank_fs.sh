#!/usr/bin/bash
#
# usage: create_blank_fs.sh <blank_img>
#
TARGET_IMAGE="$1"

IMAGE_SIZE=4G
SECTOR_SIZE=512
SECTOR_PER_CLUSTER=8


echo "create blank fat32 SD image ($IMAGE_SIZE) $TARGET_IMAGE with sectors of $CLUSTER_SIZE bits and $SECTOR_PER_CLUSTER sector per cluser"

truncate -s $IMAGE_SIZE $TARGET_IMAGE
mkfs.fat -F 32 -I -s $SECTOR_PER_CLUSTER -S $SECTOR_SIZE $TARGET_IMAGE



