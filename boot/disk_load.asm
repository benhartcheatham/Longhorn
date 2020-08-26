[bits 16]

;address to read into should be in bx, amount of sectors to read in cl, and drive to read from in dl
;returns 0 in ax if read succeeded, error code in al otherwise
disk_load:
    push dx

    mov ah, 0x02    ;setup for BIOS function
    mov al, dh      ;number of secotrs to read goes in al
    mov ch, 0       ;cylinder goes in ch
    mov dh, 0       ;head number goes in dh
    int 0x13        ;call function
    jc .disk_error  ;jump if read failed

    cmp dh, al      ;compare number of sectors read to requested number
    jne .disk_error ;jump if read didn't read requested number (failed)
    mov ax, 0       ;return in success code
    pop dx
    ret

.disk_error:
    pop dx
    shr ax, 4       ;put error code in al
    ret