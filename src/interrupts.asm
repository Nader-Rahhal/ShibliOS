[BITS 64]

section .text

global isr_stub_0
global isr_stub_1
global isr_stub_2
global isr_stub_3
global isr_stub_4
global isr_stub_5
global isr_stub_6
global isr_stub_7
global isr_stub_8
global isr_stub_9
global isr_stub_10
global isr_stub_11
global isr_stub_12
global isr_stub_13
global isr_stub_14
global isr_stub_15
global isr_stub_16
global isr_stub_17
global isr_stub_18
global isr_stub_19
global isr_stub_20
global isr_stub_21
global isr_stub_22
global isr_stub_23
global isr_stub_24
global isr_stub_25
global isr_stub_26
global isr_stub_27
global isr_stub_28
global isr_stub_29
global isr_stub_30
global isr_stub_31

extern exception_handler

%macro isr_no_err_stub 1
isr_stub_%+%1:
    push 0              ; Push dummy error code
    push %1             ; Push interrupt number
    jmp isr_common
%endmacro

%macro isr_err_stub 1
isr_stub_%+%1:
    ; CPU already pushed error code
    push %1             ; Push interrupt number
    jmp isr_common
%endmacro

isr_no_err_stub 0   ; Divide Error
isr_no_err_stub 1   ; Debug
isr_no_err_stub 2   ; NMI
isr_no_err_stub 3   ; Breakpoint
isr_no_err_stub 4   ; Overflow
isr_no_err_stub 5   ; Bound Range Exceeded
isr_no_err_stub 6   ; Invalid Opcode
isr_no_err_stub 7   ; Device Not Available
isr_err_stub 8      ; Double Fault (has error code)
isr_no_err_stub 9   ; Coprocessor Segment Overrun
isr_err_stub 10     ; Invalid TSS (has error code)
isr_err_stub 11     ; Segment Not Present (has error code)
isr_err_stub 12     ; Stack Fault (has error code)
isr_err_stub 13     ; General Protection Fault (has error code)
isr_err_stub 14     ; Page Fault (has error code)
isr_no_err_stub 15  ; Reserved
isr_no_err_stub 16  ; x87 FPU Error
isr_err_stub 17     ; Alignment Check (has error code)
isr_no_err_stub 18  ; Machine Check
isr_no_err_stub 19  ; SIMD Floating-Point Exception
isr_no_err_stub 20  ; Virtualization Exception
isr_err_stub 21     ; Control Protection Exception (has error code)
isr_no_err_stub 22  ; Reserved
isr_no_err_stub 23  ; Reserved
isr_no_err_stub 24  ; Reserved
isr_no_err_stub 25  ; Reserved
isr_no_err_stub 26  ; Reserved
isr_no_err_stub 27  ; Reserved
isr_no_err_stub 28  ; Hypervisor Injection Exception
isr_err_stub 29     ; VMM Communication Exception (has error code)
isr_err_stub 30     ; Security Exception (has error code)
isr_no_err_stub 31  ; Reserved

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

global irq_stub_0
global irq_stub_1

extern irq_handler

%macro irq_stub_macro 1
irq_stub_%+%1:
    push 0              ; Dummy error code
    push %1 + 32        ; Push IRQ number as interrupt vector (32 + IRQ)
    jmp irq_common
%endmacro

irq_stub_macro 0        ; Timer (vector 32)
irq_stub_macro 1        ; Keyboard (vector 33)


irq_common:
    ; Save registers (same as exception handler)
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
    
    mov rbp, rsp
    and rsp, ~0xF
    
    mov rdi, rbp
    cld
    call irq_handler     ; Call IRQ handler (not exception_handler!)
    
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