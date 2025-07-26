# Définir les variables
build_dir := "build"
cross_file := "raspi0.ini"

default:
  just --list

# build tasks

configure:
    meson setup {{build_dir}} --cross-file {{cross_file}}

build:
    meson compile -C {{build_dir}} -v

clean:
    rm -rf {{build_dir}}

kernel_size: build
    @echo "kernel size: $((0x$(arm-none-eabi-objdump -t build/kernel.elf | grep '_end' | sort  | cut -f1 -d' ' | sed 's/^0*//') - 0x8000))"

# emulation
run_kernel: build kernel_size
    @echo "Press ctrl+A X to quit emulation"
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/kernel.elf -d guest_errors -serial null -serial mon:stdio -s

run_kernel_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/kernel.elf -d guest_errors -serial null -serial pty -s

run_serial_loader: build
    @echo "Press ctrl+A X to quit emulation"
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/serial_loader.elf -d guest_errors -serial null -serial mon:stdio -s

run_serial_loader_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/serial_loader.elf -d guest_errors -serial null -serial pty -s

# real hardware

flash: build
    python tools/serial_boot.py --device /dev/ttyACM0 --data {{build_dir}}//kernel.img

run_kernel_raspi0: flash
    minicom --device /dev/ttyACM0 --color=on
    @echo 'reboot satan OS..'
    echo -e 'reboot\r\n' > /dev/ttyACM0
