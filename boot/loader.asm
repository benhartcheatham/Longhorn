[bits 16]
[org 0x7e00]

xor ax, ax
mov ds, ax

;requires the drive to load kernel from be in bx, and sector kernel is at on disk in cx
;sets up kernel environment and loads/executes the kernel
load:
    mov [BOOT_DRIVE], bx        ;save boot drive and sector kernel is at on disk
    mov bx, cx
    mov [KERNEL_SECTOR], bx

    call do_e820                ;scan/save memory map
    call enable_a20             ;enable the a20 line

    call load_kernel            ;load the kernel into memory
    call load_gdt               ;load the gdt into the gdt register

    jmp switch_pm              ;switch to protected mode and start kernel

    ret


;loads kernel into memory at offset KERNEL_OFFSET_32
load_kernel:
    mov ax, KERNEL_OFFSET_16
    mov es, ax

    mov bx, 0                   ;load to KERNEL_OFFSET_16:0 (KERNEL_OFFSET_32)
    mov dh, KERNEL_SIZE         ;read KERNEL_SIZE sectors
    mov dl, [BOOT_DRIVE]        ;read from drive BOOT_DRIVE
    mov cl, [KERNEL_SECTOR]     ;read from sector number KERNEL_SECTOR
    call disk_load

    cmp ax, 0
    jne .read_failure
    ret

.read_failure:
    push ax
    mov bx, KERNEL_LOAD_FAILURE ;print FAILURE string
    call print_16

    pop ax
    call print_int              ;print error code
    jmp $

;loads the gdt
load_gdt:
    cli
    lgdt [gdt_descriptor]
    ret

;switches to protected mode, is not expected to return
switch_pm:
    mov eax, cr0        ;set first bit of cr0 to 1 to enable pm
    or eax, 1
    mov cr0, eax

    jmp 0x08:init_pm   ;jump to init_pm in code segment

[bits 32]
;sets up protected mode for the kernel
init_pm:
    mov eax, 0x10       ;load data segment to segment register
    mov ds, eax
    mov ss, eax
    mov es, eax
    mov fs, eax
    mov gs, eax

    mov ebp, 0x6000     ;set up kernel stack
    mov esp, ebp

    call KERNEL_OFFSET_32  ;execute kernel

    jmp $               ;spin if kernel returns
;gdt data
gdt:
    gdt_null:
        dd 0x0          ;dd is a double-word (4 bytes)
        dd 0x0

    gdt_code:
        ;base = 0x0, limit=0xfffff
        ;1st flags: (present) 1, (privilege) 00, (descriptor type) 1
        ;type flags: (code) 1, (conforming) 0, (readable) 1, (accessed) 0
        ;2nd flags: (granularity) 1 (32-bit default) 1, (64-bit seg) 0, (AVL) 0
        dw 0xffff       ;Limit
        dw 0x0          ;Base (bits 0-15)
        db 0x0          ;Base (bits 16-23)
        db 10011010b    ;1st flags, type flags
        db 11001111b    ;2nd flags, Limit (bits 16-19)
        db 0x0          ;Base (bits 24-31)
    
    gdt_data:
        ;type flags: (code) 0, (expand down) 0, (writable) 1, (accessed) 0
        ;everything else is the same
        dw 0xffff
        dw 0x0
        db 0x0
        db 10010010b
        db 11001111b
        db 0x0
    
    gdt_end:


gdt_descriptor:
    dw gdt_end - gdt -1   ;size of GDT, always one less than true size
    dd gdt                ;start address of the GDT

;includes
%include "print.asm"
%include "disk_load.asm"
%include "e820.asm"
%include "a20.asm"

;constants
KERNEL_SIZE equ 1
KERNEL_OFFSET_16 equ 0x2000
KERNEL_OFFSET_32 equ 0x20000

;variables
HELLO: db "Hello World!", 0
BOOT_DRIVE: db 0
KERNEL_SECTOR: db 0
KERNEL_LOAD_FAILURE: db "KERNEL FAILED TO LOAD. CODE: ", 0

times 1024 - ($ - $$) db 0