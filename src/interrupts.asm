[BITS 64]

section .text

global isr_stub_0
extern exception_handler

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0              ; Push dummy error code
    push %1             ; Push interrupt number
    jmp isr_common
%endmacro

isr_no_err_stub 0

isr_common:
    ; Save all registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Align stack to 16 bytes (subtract 8 if needed for alignment)
    mov rbp, rsp        ; Save original stack pointer
    and rsp, ~0xF       ; Align to 16 bytes (clear lower 4 bits)
    
    ; Pass original stack pointer to handler
    mov rdi, rbp
    cld
    call exception_handler
    
    ; Restore stack pointer
    mov rsp, rbp
    
    ; Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    add rsp, 16
    iretq