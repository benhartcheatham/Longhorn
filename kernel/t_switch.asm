[bits 32]

global switch_threads
switch_threads:
    push ebp
    mov ebp, esp
    push ebx
    push edi
    push esi

    mov ebx, [ebp + 8]  ;access the current thread struct
    mov [ebx], esp      ;update the esp pointer of current thread struct

    mov ebx, [ebp + 12] ;access the next thread struct
    mov esp, [ebx]      ;update esp to the current thread esp

    pop esi             ;pop thread state and stack frame
    pop edi
    pop ebx
    pop ebp
    ret

global first_switch_entry
first_switch_entry:
    ;add esp, 8              ;discard switch_threads args
    extern finish_schedule
    call finish_schedule
    ret
