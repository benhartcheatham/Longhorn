C_SOURCES = $(wildcard libc/*.c kernel/*.c drivers/*.c)
HEADERS = $(wildcard libc/*.h kernel/*.h drivers/*.h)
ASM_SOURCES = $(wildcard kernel/*.asm)
BOOT_SOURCES = boot/boot.asm boot/loader.asm

OBJ = ${C_SOURCES:.c=.o}
ASM = ${ASM_SOURCES:.asm=.o}
BOOT = ${BOOT_SOURCES:.asm=.o}

CC = i386-elf-gcc
CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32

all: os-binary

run: all
	qemu-system-i386 -fda os-binary

run-img: os-img disk.img
	qemu-system-i386 -fda disk.img

run-no-reboot: all
	qemu-system-i386 -fda os-binary -no-reboot -no-shutdown

%.bin: %.asm
	nasm $< -f bin -i "./boot/" -o $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf -o $@

#Update to use linker.ld instead of -Ttext
kernel/kernel.bin: boot/kernel_entry.o ${OBJ} ${ASM}
	i386-elf-ld -Ttext 0x20000 -o $@ $^ --oformat binary

os-binary: boot/boot.bin boot/loader.bin kernel/kernel.bin
	cat boot/boot.bin boot/loader.bin > $@

#Builds a floppy img of the os
os-img: boot/boot.bin boot/loader.bin
	dd if=/dev/zero of=disk.img bs=1024 count=720
	dd if=boot/boot.bin of=disk.img conv=notrunc
	dd if=boot/loader.bin of=disk.img bs=512 seek=1 conv=notrunc
	dd if=

clean:
	rm -rf *.bin *.o
	rm -rf ./*/*.bin ./*/*.o