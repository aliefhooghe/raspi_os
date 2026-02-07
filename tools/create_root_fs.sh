#!/usr/bin/bash
#
# usage: create_root_fs.sh <target_image> <blank_img> [user programs ...]
#
TARGET_IMAGE="$1"
shift
BLANK_IMAGE="$1"
shift

echo "initialize target image $TARGET_IMAGE from blank $BLANK_IMAGE"
cp $BLANK_IMAGE $TARGET_IMAGE

echo "create rootfs layout"
mmd -i "$TARGET_IMAGE" ::/bin
mmd -i "$TARGET_IMAGE" ::/dev
mmd -i "$TARGET_IMAGE" ::/data

for f in "$@"; do
    echo "copy binary $f"
    mcopy -i "$TARGET_IMAGE" "$f" ::/bin/$(basename "$f" | sed -e 's/\.elf//')
done
