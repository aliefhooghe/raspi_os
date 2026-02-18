# Définir les variables
build_dir := "build"
cross_file := "raspi0.ini"

default:
  just --list

## build tasks

# configure build directory
configure:
    meson setup {{build_dir}} --cross-file {{cross_file}}

# build satan OS and userland programs
build:
    meson compile -C {{build_dir}} -v

# remove build directory
clean:
    rm -rf {{build_dir}}

# display the kernel kernel_size
kernel_size: build
    @echo "kernel size: $(echo $((0x$(arm-none-eabi-objdump -t build/kernel.elf | grep '_end' | sort  | cut -f1 -d' ' | sed 's/^0*//') - 0x8000)) | numfmt --to=iec --suffix=B)"

# 
kernel_bloaty: build
    # vm: size when loaded in ram
    bloaty build/kernel.elf -d compileunits,symbols --domain vm  -s vm -n 0
    
# run kernel emulation
run_kernel: build
    @echo "Press ctrl+A X to quit emulation"
    qemu-system-arm -display none -m 512 -M raspi0 \
        -kernel {{build_dir}}/kernel.elf -d guest_errors \
        -serial null -serial mon:stdio -s \
         -drive if=sd,file=./build/sd.rootfs.img,format=raw

# run kernel emulation with remote debug adapter
debug_kernel: build kernel_size
    @echo "Press ctrl+A X to quit emulation"
    qemu-system-arm -display none -m 512 -M raspi0 \
        -kernel {{build_dir}}/kernel.elf -d guest_errors \
        -serial null -serial mon:stdio  \
        -drive if=sd,file=./build/sd.rootfs.img,format=raw \
        -gdb tcp::5000 -S

gdb:
    arm-none-eabi-gdb build/kernel.elf -ex "target remote :5000"
    
gdb-probe:
    arm-none-eabi-gdb build/kernel.elf -ex "target remote :5000"

run_kernel_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel \
    {{build_dir}}/kernel.elf -d guest_errors \
    -serial null -serial pty

run_serial_loader_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel \
    {{build_dir}}/serial_loader.elf -d guest_errors \
    -serial null -serial pty \
    -drive if=sd,file=./build/sd.rootfs.img,format=raw

# flash the kernel to serial loader
flash: build
    uv run tools/serial_boot.py --device /dev/ttyACM0 --data {{build_dir}}//kernel.img

run_kernel_raspi0: flash
    minicom --device /dev/ttyACM0 --color=on
    @echo 'reboot satan OS..'
    echo -e 'reboot\r\n' > /dev/ttyACM0

# run a terminal emulator attached to the real harware
tty_raspi:
    minicom --device /dev/ttyACM0 --color=on
