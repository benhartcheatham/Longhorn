# Longhorn
A rewrite of my original toy OS that is now using GRUB2 over a custom bootloader

# Resources Used
https://wiki.osdev.org/Bare_Bones

# Formatting Files
.c:
    - Functions should be declared in the order that they are declared in the .h file (if applicable)
    - Global variables should be before function implementations
    - Includes should be at the top of the file
        - std library includes should be first
        - includes should be grouped based on file path (ex: ../drivers/ and ../libc/ files should be grouped seperately)
.h:
    - Each .h file should have an include guard
    - Each section should be denoted by a comment before the section starts and should be in the order below
    - Sections
        - includes
        - defines
        - structs 
            - enums, unions, etc. should also be defined here
        - typedefs
        - functions
            - highly encouraged to group functions based on what they do (e.g. functions for modifying mem grouped together)