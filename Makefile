### MAKE GROUPS ###
SUBDIRS = drivers kernel libc libk

C_SOURCES = $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.c))
HEADERS = $(foreach dir, $(SUBDIRS), $(wildcard $(dir)/*.h))
LIBINCLUDE = include/libc include/libk
ASM_SOURCES = $(wildcard kernel/*.asm)
BOOT_SOURCES = boot/boot.asm

ASSETS = $(wildcard assets/*.bmp)

OBJ = $(C_SOURCES:.c=.o) $(ASSETS:.bmp=.o)
ASM = $(ASM_SOURCES:.asm=.o)
BOOT = $(BOOT_SOURCES:.asm=.o)

### COMPILER GROUPS/RULES ###
CC = i686-elf-gcc
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
CPPFLAGS = $(foreach dir, $(LIBINCLUDE), -I "$(dir)")
QEMU = qemu-system-i386
DEFINES = 

### INSTALLATION GROUPS ###
# BINUTILS_VER = 2.24
# GCC_VER = 4.9.1
# OS = "Arch"
# BUILD_DIR = ""

### MAKEFILE RULES ###

### EXECUTE RULES ###
all: os-binary os-img

# default is to run iso
run: all
	$(QEMU) -cdrom Longhorn.iso

# runs ab usi with no restart on crash
run-no-reboot: all
	$(QEMU) -cdrom Longhorn.iso -no-reboot -no-shutdown

### BUILD RULES ###

# runs a version of the kernel that has testing enabled
test: DEFINES += -D TESTS # look at https://www.gnu.org/software/make/manual/make.html#Target_002dspecific for how this works
test: run

# cleans all directories of compiled files (except build)
clean:
	rm -rf *.bin *.o *.iso
	rm -rf ./*/*.bin ./*/*.o
	rm os-binary

### FILE SPECIFIC RULES ###
boot/boot.o: boot/boot.asm
	nasm $< -f elf32 -i "./boot/" -o $@

os-binary: $(BOOT) $(OBJ) $(ASM)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFINES) -T linker.ld -o $@ $^ -nostdlib -lgcc

### FILE EXTENSION RULES ###
%.bin: %.asm
	nasm $< -f bin -i -o $@

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFINES) -c $< -o $@

%.o: %.asm
	nasm $< -f elf32 -o $@

%.o: %.bmp
	$ objcopy --input binary --output elf32-i386 --binary-architecture i386 \
    $< $@

# Builds a disk img of the os
os-img:
	mkdir -p build/boot/grub
	cp grub.cfg build/boot/grub/grub.cfg
	cp os-binary build/boot/os-binary
	grub-mkrescue -o Longhorn.iso build

### INSTALL RULES ###

# install:
# 	-@if [ -n $(BUILD_DIR) ]; then 															\
# 		./install.sh -o $(OS) -b $(BINUTILS_VER) -g $(GCC_VER) -d $(BUILD_DIR) -s;   \
# 	else																					\
# 		./install.sh -o $(OS) -b $(BINUTILS_VER) -g $(GCC_VER) -s; 					\
# 	fi

# depend:
#   -@if [ $OS = "Ubuntu" ]; then \
# 		echo "this DOES NOT install grub components. You will need to install the packages for" \
# 		echo "grub-mkrescue (grub-common) and xorriso for make to work." \
# 		apt install grub-common qemu-system-x86 nasm	\
#     else if [ $OS = "Arch" ]; then \
# 		echo "This DOES NOT install grub. If you don't have the grub package, you will need it" \
# 		sudo pacman -S qemu qemu-arch-extra nasm libisoburn mtools \
# 	fi

# help:
# 	-@if [ $(OS) = "Arch" ]; then \
# 		echo -e " To compile the OS, run make. To compile and run, run make run. Finally, to compile and run the testing suite run make test.\n" \
# 		"To compile you will need an i686 cross compiler (made in make install) aliased to i686-elf-gcc, grub-mkrescue, and a qemu\n"   \
# 		"installation that can emulate i386 aliased to qemu-system-i386. You can install these dependencies (on systems with apt) by running make depend.\n"														   \
# 		"To create an i686 cross compiler compatible with this makefile, run make install. Options for installation include:\n"		   \
# 		"\tGCC_VER: Version of gcc of the compiler\n\tBINUTILS_VER: Version of binutils of the compiler\n"					   \
# 		"\tOS: The OS you are using to compile the corss compiler. MUST CHANGE FROM \"Ubuntu\" IF NOT ON A DEBIAN SYSTEM.\n"         \
# 		"\tSupported OS archetypes are: Ubuntu, Arch.\n"                       													   \
# 		"\tBUILD_DIR: Directory to build the compiler in.\n To set any of these variables do <VAR_NAME>=<VALUE>.\n"                   \
# 		"If you don't want to use aliases, the makefile can be changed under the COMPILER GROUPS/RULES section to use your commands."; \
# 	else \
# 		echo " To compile the OS, run make. To compile and run, run make run. Finally, to compile and run the testing suite run make test.\n" \
# 		"To compile you will need an i686 cross compiler (made in make install) aliased to i686-elf-gcc, grub-mkrescue, and a qemu\n"   \
# 		"installation that can emulate i386 aliased to qemu-system-i386. You can install these dependencies (on systems with apt) by running make depend.\n"														   \
# 		"To create an i686 cross compiler compatible with this makefile, run make install. Options for installation include:\n"		   \
# 		"\tGCC_VER: Version of gcc of the compiler\n\tBINUTILS_VER: Version of binutils of the compiler\n"					   \
# 		"\tOS: The OS you are using to compile the corss compiler. MUST CHANGE FROM \"Ubuntu\" IF NOT ON A DEBIAN SYSTEM.\n"         \
# 		"\tSupported OS archetypes are: Ubuntu, Arch.\n"                       													   \
# 		"\tBUILD_DIR: Directory to build the compiler in.\n To set any of these variables do <VAR_NAME>=<VALUE>.\n"                   \
# 		"If you don't want to use aliases, the makefile can be changed under the COMPILER GROUPS/RULES section to use your commands."; \
# 	fi