#include <stdbool.h>
#include <stdint.h>

#include <serial.h>
#define MAX_NUM_IDT_ENTRIES 256

static inline void enable_interrupts(void) {
    asm volatile ("sti");
}

static inline void disable_interrupts(void) {
    asm volatile ("cli");
}


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
} idt_entry_t __attribute__((packed));

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));


__attribute__((used)) static idt_entry_t idt[256];

extern void isr_stub_0(void);

static inline void set_idt_entry(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];
    uint64_t isr_addr = (uint64_t)isr;

    descriptor->isr_low   = isr_addr & 0xFFFF;
    descriptor->selector  = 0x28;
    descriptor->ist       = 0;    
    descriptor->attributes = flags;
    descriptor->isr_mid   = (isr_addr >> 16) & 0xFFFF;
    descriptor->isr_high  = (isr_addr >> 32) & 0xFFFFFFFF;
    descriptor->reserved  = 0;
}

// this will handle 
void exception_handler(struct interrupt_frame *frame) {
    serial_write("\n=== EXCEPTION ===\n");

    if (frame->int_no == 0) {
        serial_write("Divide By Zero Exception\n");
    } else {
        serial_write("Unknown Exception\n");
    }
    
    while (1) {
        asm volatile ("hlt");
    }
}

static inline void idt_init(void) {
    
    for (int i = 0; i < 256; i++) {
        idt[i].isr_low = 0;
        idt[i].selector = 0;
        idt[i].ist = 0;
        idt[i].attributes = 0;
        idt[i].isr_mid = 0;
        idt[i].isr_high = 0;
        idt[i].reserved = 0;
    }

    set_idt_entry(0, isr_stub_0, 0x8E);
    
    struct idtr idtr;
    idtr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idtr.base = (uint64_t)&idt;
    
    asm volatile("lidt %0" :: "m"(idtr));
    
    serial_write("IDT loaded!\n");
}