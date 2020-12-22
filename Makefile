### GROUPS ###
SUBDIRS = drivers kernel libc libk
C_SOURCES = $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.c))
HEADERS = $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.h))
ASM_SOURCES = $(wildcard kernel/*.asm)
BOOT_SOURCES = boot/boot.asm
ASSETS = $(wildcard assets/*.bmp)

OBJ = $(C_SOURCES:.c=.o) $(ASSETS:.bmp=.o)
ASM = $(ASM_SOURCES:.asm=.o)
BOOT = $(BOOT_SOURCES:.asm=.o)

### COMPILER GROUPS/RULES ###
CC = i386-elf-gcc
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
DEFINES = 

### MAKEFILE RULES ###
all: os-binary os-img

#default is to run iso
run: all
	qemu-system-i386 -cdrom Longhorn.iso

#runs ab usi with no restart on crash
run-no-reboot: all
	qemu-system-i386 -cdrom Longhorn.iso -no-reboot -no-shutdown

#runs a version of the kernel that has testing enabled
test: DEFINES += -DTESTS		#look at https://www.gnu.org/software/make/manual/make.html#Target_002dspecific for how this works
test: run

#cleans all directories of compiled files (except build)
clean:
	rm -rf *.bin *.o *.iso
	rm -rf ./*/*.bin ./*/*.o
	rm os-binary

### FILE SPECIFIC RULES ###
boot/boot.o: boot/boot.asm
	nasm $< -f elf32 -i "./boot/" -o $@

os-binary: $(BOOT)  $(OBJ) $(ASM)
	$(CC) $(CFLAGS) $(DEFINES) -T linker.ld -o $@ $^ -nostdlib -lgcc

### FILE EXTENSION RULES ###
%.bin: %.asm
	nasm $< -f bin -i -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

%.o: %.bmp
	$ objcopy --input binary --output elf32-i386 --binary-architecture i386 \
    $< $@

### BUILD RULES ###

#Builds a disk img of the os
os-img:
	mkdir -p build/boot/grub
	cp grub.cfg build/boot/grub/grub.cfg
	cp os-binary build/boot/os-binary
	grub-mkrescue -o Longhorn.iso build
