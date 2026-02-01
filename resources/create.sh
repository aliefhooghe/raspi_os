
dd if=/dev/zero of=fat32.img bs=512 count=200


# 8 sector per cluster
# 512 bytes per sector
mkfs.fat -F 32 -I -s 8 -S 512 fat32.img
