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
    