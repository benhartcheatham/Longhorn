print_16:
    push bx
    mov ah, 0x0e
    
    .loop:
        mov al, [bx]
        int 0x10
        cmp al, 0
        je .return
        inc bx
        jmp .loop

    .return:
        ;insert a newline and carriage return
        mov al, 10
        int 0x10
        mov al, 13
        int 0x10

        pop bx
        ret

;int to print should be in ax
print_int:
    push ax             ;push registers used
    push cx
    push dx

    .print_loop:
        xor dx, dx      ;clear dx for div
        mov cx, 10      ;div by 10 (has to be at a mem location or in a register)
        div cx
        add dx, '0'     ;convert remainder to ascii
        
        push ax
        mov ah, 0x0e    ;0x0e is BIOS print code
        mov al, dl      ;remainder of bx / 10 is in dx
        int 0x10        ;print al
        pop ax
        
        cmp ax, 0       ;print next digit if bx isn't 0
        jne .print_loop
        
        pop dx          ;pop registers used
        pop cx
        pop ax
        ret

;the address of the hex should be stored in ax
print_hex:
    push ax
    push cx
    push bx

    mov bx, hex_string
    add bx, 5
    .loop2:
        mov cx, 0x000f          ;lookup hex char
        and cx, ax
        call bin_to_hex
        mov [bx], cl            ;write char to string
        shr ax, 4               ;move to next hex char
        dec bx
        mov cx, bx
        cmp cx, hex_string + 1  ;if we are at x, stop
        jne .loop2

    mov bx, hex_string
    call print_16

    pop bx
    pop cx
    pop ax
    ret

;helper for print_hex. cx should store the hex char to print
bin_to_hex:
    push bx
    mov bx, cx
    add bx, hex_table
    mov cl, [bx]
    pop bx
    ret


hex_table: db "0123456789abcdef"
;print the string, then a new line and carriage return
hex_string: db "0x0000",0