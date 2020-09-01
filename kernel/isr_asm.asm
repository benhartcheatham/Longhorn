;CODE TAKEN FROM https://github.com/cfenollosa/os-tutorial, TUTORIALS 18 & 19
;SOME CHANGES MADE THAT ARE OUTLINED IN https://wiki.osdev.org/James_Molloy%27s_Tutorial_Known_Bugs AND https://github.com/cfenollosa/os-tutorial/tree/master/23-fixes

; Defined in isr.c
[extern isr_handler]
[extern irq_handler]

; Common ISR code
isr_common_stub:
    ; 1. Save CPU state
	pusha           ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax
	mov ax, ds      ; Lower 16-bits of eax = ds.
	push eax        ; save the data segment descriptor
	mov ax, 0x10    ; kernel data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
    ; 2. Call C handler
    cld
    push esp
	call isr_handler
	pop eax

    ; 3. Restore state
	pop eax 
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	popa
	add esp, 8      ; Cleans up the pushed error code and pushed ISR number
	iret            ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

; Common IRQ code. Identical to ISR code except for the 'call' 
; and the 'pop ebx'
irq_common_stub:
    pusha 
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call irq_handler    ; Different than the ISR code
    pop eax
    pop ebx             ; Different than the ISR code
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    popa
    add esp, 8
    iret 
	
; We don't get information about which interrupt was caller
; when the handler is run, so we will need to have a different handler
; for every interrupt.
; Furthermore, some interrupts push an error code onto the stack but others
; don't, so we will push a dummy error code for those which don't, so that
; we have a consistent stack for all of them.

; First make the ISRs global
global isr00
global isr01
global isr02
global isr03
global isr04
global isr05
global isr06
global isr07
global isr08
global isr09
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

; IRQs
global irq00
global irq01
global irq02
global irq03
global irq04
global irq05
global irq06
global irq07
global irq08
global irq09
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

; 0: Divide By Zero Exception
isr00:
    push byte 0
    push byte 0
    jmp isr_common_stub

; 1: Debug Exception
isr01:
    push byte 0
    push byte 1
    jmp isr_common_stub

; 2: Non Maskable Interrupt Exception
isr02:
    push byte 0
    push byte 2
    jmp isr_common_stub

; 3: Int 3 Exception
isr03:
    push byte 0
    push byte 3
    jmp isr_common_stub

; 4: INTO Exception
isr04:
    push byte 0
    push byte 4
    jmp isr_common_stub

; 5: Out of Bounds Exception
isr05:
    push byte 0
    push byte 5
    jmp isr_common_stub

; 6: Invalid Opcode Exception
isr06:
    push byte 0
    push byte 6
    jmp isr_common_stub

; 7: Coprocessor Not Available Exception
isr07:
    push byte 0
    push byte 7
    jmp isr_common_stub

; 8: Double Fault Exception (With Error Code!)
isr08:
    push byte 8
    jmp isr_common_stub

; 9: Coprocessor Segment Overrun Exception
isr09:
    push byte 0
    push byte 9
    jmp isr_common_stub

; 10: Bad TSS Exception (With Error Code!)
isr10:
    push byte 10
    jmp isr_common_stub

; 11: Segment Not Present Exception (With Error Code!)
isr11:
    push byte 11
    jmp isr_common_stub

; 12: Stack Fault Exception (With Error Code!)
isr12:
    push byte 12
    jmp isr_common_stub

; 13: General Protection Fault Exception (With Error Code!)
isr13:
    push byte 13
    jmp isr_common_stub

; 14: Page Fault Exception (With Error Code!)
isr14:
    push byte 14
    jmp isr_common_stub

; 15: Reserved Exception
isr15:
    push byte 0
    push byte 15
    jmp isr_common_stub

; 16: Floating Point Exception
isr16:
    push byte 0
    push byte 16
    jmp isr_common_stub

; 17: Alignment Check Exception (Uses Error Code)
isr17:
    push byte 17
    jmp isr_common_stub

; 18: Machine Check Exception
isr18:
    push byte 0
    push byte 18
    jmp isr_common_stub

; 19: Reserved
isr19:
    push byte 0
    push byte 19
    jmp isr_common_stub

; 20: Reserved
isr20:
    push byte 0
    push byte 20
    jmp isr_common_stub

; 21: Reserved  (Uses Error Code)
isr21:
    push byte 21
    jmp isr_common_stub

; 22: Reserved
isr22:
    push byte 0
    push byte 22
    jmp isr_common_stub

; 23: Reserved
isr23:
    push byte 0
    push byte 23
    jmp isr_common_stub

; 24: Reserved
isr24:
    push byte 0
    push byte 24
    jmp isr_common_stub

; 25: Reserved
isr25:
    push byte 0
    push byte 25
    jmp isr_common_stub

; 26: Reserved
isr26:
    push byte 0
    push byte 26
    jmp isr_common_stub

; 27: Reserved
isr27:
    push byte 0
    push byte 27
    jmp isr_common_stub

; 28: Reserved
isr28:
    push byte 0
    push byte 28
    jmp isr_common_stub

; 29: Reserved
isr29:
    push byte 0
    push byte 29
    jmp isr_common_stub

; 30: Reserved
isr30:
    push byte 0
    push byte 30
    jmp isr_common_stub

; 31: Reserved
isr31:
    push byte 0
    push byte 31
    jmp isr_common_stub

; IRQ handlers
irq00:
	push byte 0
	push byte 32
	jmp irq_common_stub

irq01:
	push byte 1
	push byte 33
	jmp irq_common_stub

irq02:
	push byte 2
	push byte 34
	jmp irq_common_stub

irq03:
	push byte 3
	push byte 35
	jmp irq_common_stub

irq04:
	push byte 4
	push byte 36
	jmp irq_common_stub

irq05:
	push byte 5
	push byte 37
	jmp irq_common_stub

irq06:
	push byte 6
	push byte 38
	jmp irq_common_stub

irq07:
	push byte 7
	push byte 39
	jmp irq_common_stub

irq08:
	push byte 8
	push byte 40
	jmp irq_common_stub

irq09:
	push byte 9
	push byte 41
	jmp irq_common_stub

irq10:
	push byte 10
	push byte 42
	jmp irq_common_stub

irq11:
	push byte 11
	push byte 43
	jmp irq_common_stub

irq12:
	push byte 12
	push byte 44
	jmp irq_common_stub

irq13:
	push byte 13
	push byte 45
	jmp irq_common_stub

irq14:
	push byte 14
	push byte 46
	jmp irq_common_stub

irq15:
	push byte 15
	push byte 47
	jmp irq_common_stub