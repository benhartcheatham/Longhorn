C_SOURCES = $(wildcard libc/*.c kernel/*.c drivers/*.c)
HEADERS = $(wildcard libc/*.h kernel/*.h drivers/*.h)
ASM_SOURCES = $(wildcard kernel/*.asm)
BOOT_SOURCES = boot/boot.asm

OBJ = ${C_SOURCES:.c=.o}
ASM = ${ASM_SOURCES:.asm=.o}
BOOT = ${BOOT_SOURCES:.asm=.o}

CC = i686-elf-gcc
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

all: os-img

#default is to run os-binary
run: all
	qemu-system-i386 -cdrom Longhorn.iso

#runs a flat binary with no restart on crash
run-no-reboot: all
	qemu-system-i386 -fda os-binary -no-reboot -no-shutdown

%.bin: %.asm
	nasm $< -f bin -i "./boot/" -o $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -i "./boot/" -o $@

#Update to use linker.ld instead of -Ttext
kernel/kernel.bin: ${BOOT} ${OBJ} ${ASM}
	${CC} -T linker.ld -o os-binary -ffreestanding -O2 -nostdlib $^ -lgcc

#Builds a disk img of the os
os-img: kernel/kernel.bin
	cp os-binary build/boot/os-binary
	grub-mkrescue -o Longhorn.iso build
	

#cleans all directories of compiled files
clean:
	rm -rf *.bin *.o
	rm -rf ./*/*.bin ./*/*.o