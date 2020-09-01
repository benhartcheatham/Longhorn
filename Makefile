### GROUPS ###
C_SOURCES = $(wildcard libc/*.c kernel/*.c drivers/*.c)
HEADERS = $(wildcard libc/*.h kernel/*.h drivers/*.h)
ASM_SOURCES = $(wildcard kernel/*.asm)
BOOT_SOURCES = boot/boot.asm

OBJ = ${C_SOURCES:.c=.o}
ASM = ${ASM_SOURCES:.asm=.o}
BOOT = ${BOOT_SOURCES:.asm=.o}

### COMPILER GOURPS/RULES ###
CC = i686-elf-gcc
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

### MAKEFILE RULES ###
all: os-img

#default is to run iso
run: all
	qemu-system-i386 -cdrom Longhorn.iso

#runs ab usi with no restart on crash
run-no-reboot: all
	qemu-system-i386 -cdrom Longhorn.iso -no-reboot -no-shutdown

#cleans all directories of compiled files (except build)
clean:
	rm -rf *.bin *.o *.iso
	rm -rf ./*/*.bin ./*/*.o
	rm os-binary

### FILE SPECIFIC RULES ###
boot/boot.o: boot/boot.asm
	nasm $< -f elf32 -i "./boot/" -o $@

kernel/kernel.bin: ${BOOT} ${OBJ} ${ASM}
	${CC} -T linker.ld -o os-binary -ffreestanding -O2 -nostdlib $^ -lgcc

### FILE EXTENSION RULES ###
%.bin: %.asm
	nasm $< -f bin -i -o $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

### BUILD RULES ###
#Builds a disk img of the os
os-img: kernel/kernel.bin
	cp os-binary build/boot/os-binary
	grub-mkrescue -o Longhorn.iso build