[bits 32]

global switch_threads
switch_threads:
    push ebp            ;create a stack frame
    mov ebp, esp
    pushad              ;push thread state

    mov ebx, [ebp + 8]  ;access the current thread struct
    mov [ebx], esp      ;update the esp pointer of current thread struct

    mov ebx, [ebp + 12] ;access the next thread struct
    mov [esp], ebx      ;update esp to the current thread esp

    popad               ;pop thread state
    pop ebp             ;pop stack frame
    ret

