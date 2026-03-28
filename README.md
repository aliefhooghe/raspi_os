
# Toy Raspberry pi zero W Unix type OS in pure C / asm 

- userland/kernel separation with virtual memory management
- basic round robin preemptive scheduler
- virtual file system abstraction:
  - fat32 partition (ro) mounted from sdcard at /
  - ramfs mounted at /dev with device files
- serial bootloader: can boot from mini UART


