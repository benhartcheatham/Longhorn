# Longhorn
Longhorn is a toy 32-bit x86 OS written by me in an attempt to learn a bit more about OS development and how
monolithic kernels/OSes work.

# Current Features
- Physical Memory Manager
- Threading/Processes
- Rudimentary Shell

# Compiling and Running
The make file expects to be able to use grub-mkrescue and a i686 cross-compiler for compilation.
If you have aliasing issues for the cross compiler, set CC under the COMPILER GROUPS/RULES section of the makefile to point to
your cross compiler installation.

To run, the makefile expects an installation of qemu for i386.
If you have aliasing issues, update QEMU under the COMPILER GROUPS/RULES section of the makefile to point to your QEMU installation.

Once you meet those requisites, make compiles the OS, make run compiles and runs the OS,
and make test compiles and runs the OS with testing enabled.

make clean will clean all executable files and removes the OS binary

# Resources Used
https://wiki.osdev.org/Bare_Bones
https://wiki.osdev.org/Main_Page
