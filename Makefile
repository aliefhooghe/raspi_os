
all: build/kernel7.img

build/boot.o: boot.S
	arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -c boot.S -o build/boot.o

build/kernel.o: kernel.c
	arm-none-eabi-gcc -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu99 -c kernel.c -O2 -Wall -Wextra -o build/kernel.o

build/kernel.elf: linker.ld build/boot.o build/kernel.o
	arm-none-eabi-gcc -T linker.ld -ffreestanding -O2 -nostdlib build/boot.o build/kernel.o -lgcc -o kernel.elf -o build/kernel.elf

build/kernel7.img: build/kernel.elf
	arm-none-eabi-objcopy build/kernel.elf -O binary build/kernel7.img

run: build/kernel.elf
	qemu-system-arm -m 512 -M raspi0 -serial stdio -kernel build/kernel.elf

clean:
	rm build/*
