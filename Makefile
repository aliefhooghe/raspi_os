
all: build/build/compile_commands.json build/kernel7.img

build/boot.o: src/boot.S
	clang --target=armv6m-none-eabi -mcpu=arm1176jzf-s -fpic -ffreestanding -c src/boot.S -o build/boot.o

build/kernel.o: src/kernel.c
	clang --target=armv6m-none-eabi -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c src/kernel.c -O2 -Wall -Wextra -o build/kernel.o

build/kernel.elf: src/linker.ld build/boot.o build/kernel.o
	clang --target=armv6m-none-eabi -T src/linker.ld -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o -o kernel.elf -o build/kernel.elf

build/kernel7.img: build/kernel.elf
	llvm-objcopy build/kernel.elf -O binary build/kernel7.img

build/build/compile_commands.json: Makefile
	python tools/generate_clangd_db.py --makefile Makefile --commands build/compile_commands.json

run: build/kernel.elf
	qemu-system-arm -m 512 -M raspi0 -serial mon:stdio -kernel build/kernel.elf

clean:
	rm build/*
