# Définir les variables
build_dir := "build"
cross_file := "raspi0.ini"

default:
  just --list

configure:
    meson setup {{build_dir}} --cross-file {{cross_file}}

build:
    meson compile -C {{build_dir}}

run_kernel: build
    @echo "Press ctrl+A X to quit emulation"
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/kernel.elf -d guest_errors -serial null -serial mon:stdio

run_kernel_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/kernel.elf -d guest_errors -serial null -serial pty

run_serial_loader: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/serial_loader.elf -d guest_errors -serial null -serial mon:stdio

run_serial_loader_pty: build
    qemu-system-arm -display none -m 512 -M raspi0 -kernel {{build_dir}}/serial_loader.elf -d guest_errors -serial null -serial pty

clean:
    rm -rf {{build_dir}}
