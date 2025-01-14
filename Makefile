TARGET_ABI = armv6m-none-eabi
TARGET_CPU = arm1176jzf-s

CFLAGS   = --target=$(TARGET_ABI) -mcpu=$(TARGET_CPU) -fpic -ffreestanding -O2 -Wall -Wextra -nostdlib



all: build/build/compile_commands.json build/kernel.img

build/boot.o: src/kernel/boot.S
	clang $(CFLAGS) -c src/kernel/boot.S -o build/boot.o

build/kernel.o: src/kernel/kernel.c
	clang $(CFLAGS) -c src/kernel/kernel.c -o build/kernel.o

build/kernel.elf: src/linker.ld build/boot.o build/kernel.o
	clang $(CFLAGS) -T src/linker.ld build/boot.o build/kernel.o -o kernel.elf -o build/kernel.elf

build/kernel.img: build/kernel.elf
	llvm-objcopy build/kernel.elf -O binary build/kernel.img

build/build/compile_commands.json: Makefile
	python tools/generate_clangd_db.py --makefile Makefile --commands build/compile_commands.json

run: build/kernel.elf
	# enable aux uart on stdio
	@echo "Press ctrl+A X to quit emulation"
	qemu-system-arm -display none -m 512 -M raspi0 -kernel build/kernel.elf -d guest_errors -serial null -serial mon:stdio

run_pty: build/kernel.elf
	qemu-system-arm -display none -m 512 -M raspi0 -kernel build/kernel.elf -d guest_errors -serial null -serial pty

clean:
	rm build/*
