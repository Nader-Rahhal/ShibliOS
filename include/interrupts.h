#pragma once
#include <stdbool.h>
#include <stdint.h>


#define MAX_NUM_IDT_ENTRIES 256

extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);

extern void irq_stub_0(void);
extern void irq_stub_1(void);
extern void irq_stub_8(void);
extern void irq_stub_12(void);
extern void irq_stub_14(void);
extern void irq_stub_15(void);

static void* isr_stubs[32] = {
    isr_stub_0,  isr_stub_1,  isr_stub_2,  isr_stub_3,
    isr_stub_4,  isr_stub_5,  isr_stub_6,  isr_stub_7,
    isr_stub_8,  isr_stub_9,  isr_stub_10, isr_stub_11,
    isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15,
    isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19,
    isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23,
    isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27,
    isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31
};


struct idtr {
    uint16_t limit;    // Size of IDT - 1
    uint64_t base;     // Address of IDT
} __attribute__((packed));

typedef struct {
    uint16_t isr_low;      // Lower 16 bits of ISR address
    uint16_t selector;     // Code segment selector
    uint8_t  ist;          // Interrupt Stack Table offset
    uint8_t  attributes;   // Type and attributes
    uint16_t isr_mid;      // Middle 16 bits of ISR address
    uint32_t isr_high;     // Upper 32 bits of ISR address
    uint32_t reserved;     // Reserved, must be 0
} idt_entry_t;

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));


__attribute__((used)) static idt_entry_t idt[256];

void enable_interrupts(void);

void disable_interrupts(void);

void set_idt_entry(uint8_t vector, void* isr, uint8_t flags);

void exception_handler(struct interrupt_frame *frame);

void irq_handler(struct interrupt_frame *frame);

void idt_init(void);
