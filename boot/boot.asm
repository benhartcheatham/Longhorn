[bits 16]
[org 0x7c00]

mov [BOOT_DRIVE], dl    ;save boot drive at BOOT_DRIVE

mov bp, 0x7000          ;setup a temporary stack
mov sp, bp

boot:
    xor ax, ax                  ;set es for correct addressing
    mov es, ax

    mov bx, LOADER_OFFSET       ;load 2nd stage to 0x7e00
    mov dh, SECOND_STAGE_SIZE   ;read 2 sectors
    mov dl, [BOOT_DRIVE]        ;read from drive BOOT_DRIVE
    mov cl, 2                   ;read from sector number 2
    call disk_load

    cmp ax, 0                   ;see if read failed
    jne .read_failure

    mov bx, BOOT_DRIVE          ;put BOOT_DRIVE in bx for 2nd stage of loader
    mov cx, KERNEL_START_SECTOR ;put KERNEL_START_SECTOR in cx for 2nd stage of loader
    call 0:LOADER_OFFSET

    jmp $
.read_failure:
    push ax
    mov bx, FAILURE     ;print FAILURE string
    call print_16

    pop ax
    call print_int      ;print error code
    jmp $

;includes
%include "disk_load.asm"
%include "print.asm"

;constants
LOADER_OFFSET equ 0x7e00
SECOND_STAGE_SIZE equ 2
KERNEL_START_SECTOR equ 2 + SECOND_STAGE_SIZE
;variables
BOOT_DRIVE: db 0
FAILURE: db "FAILED TO READ DRIVE. CODE: ", 0

times 510 - ($ - $$) db 0
dw 0xaa55