C_SOURCES = $(wildcard lib/*.c kernel/*.c drivers/*.c)
HEADERS = $(wildcard lib/*.h kernel/*.h drivers/*.h)
ASM_SOURCES = $(filter-out boot/boot.asm, $(wildcard boot/*.asm))

OBJ = ${C_SOURCES:.c=.o}
ASM = ${ASM_SOURCES:.asm=.o}

CFLAGS = -g -ffreestanding -Wall -Wextra -fno-exceptions -m32

all: os-binary

run: all
	qemu-system-i386 -fda os-binary

run-no-reboot: all
	qemu-system-i386 -fda os-binary -no-reboot -no-shutdown

%.o: %.asm
	nasm $< -f bin -i "./boot/" -o $@

os-binary: boot/boot.o boot/loader.o
	cat boot/boot.o boot/loader.o > os-binary