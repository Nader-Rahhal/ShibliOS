#pragma once
#include <stdbool.h>
#include <stdint.h>

#include <serial.h>
#include <pic.h>
#include <draw.h>       
#include <font.h>      
#include <keyboard.h>
#include <paging.h>
#include <rtc.h>



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
} idt_entry_t;

struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));


__attribute__((used)) static idt_entry_t idt[256];

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
    disable_interrupts();
    
    serial_write("\n\n");
    serial_write("===   EXCEPTION TRIGGERED!   ===\n");
    
    serial_write("Exception Vector: ");
    serial_write_dec(frame->int_no);
    serial_write(" (0x");
    serial_write_hex(frame->int_no);
    serial_write(")\n");
    
    const char* exception_names[] = {
        "Divide by Zero",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 FPU Error",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception"
    };
    
    if (frame->int_no < 22) {
        serial_write("Name: ");
        serial_write(exception_names[frame->int_no]);
        serial_write("\n");
    }
    
    serial_write("\n--- CPU State ---\n");
    serial_write("Error Code:  ");
    serial_write_hex(frame->error_code);
    serial_write("\n");
    
    serial_write("RIP:         ");
    serial_write_hex(frame->rip);
    serial_write("\n");
    
    serial_write("CS:          ");
    serial_write_hex(frame->cs);
    serial_write("\n");
    
    serial_write("RFLAGS:      ");
    serial_write_hex(frame->rflags);
    serial_write("\n");
    
    serial_write("RSP:         ");
    serial_write_hex(frame->rsp);
    serial_write("\n");
    
    serial_write("SS:          ");
    serial_write_hex(frame->ss);
    serial_write("\n");
    
    serial_write("\n--- General Purpose Registers ---\n");
    serial_write("RAX: ");
    serial_write_hex(frame->rax);
    serial_write("  RBX: ");
    serial_write_hex(frame->rbx);
    serial_write("\n");
    
    serial_write("RCX: ");
    serial_write_hex(frame->rcx);
    serial_write("  RDX: ");
    serial_write_hex(frame->rdx);
    serial_write("\n");
    
    serial_write("RSI: ");
    serial_write_hex(frame->rsi);
    serial_write("  RDI: ");
    serial_write_hex(frame->rdi);
    serial_write("\n");
    
    serial_write("RBP: ");
    serial_write_hex(frame->rbp);
    serial_write("\n");
    
    serial_write("\nR8:  ");
    serial_write_hex(frame->r8);
    serial_write("  R9:  ");
    serial_write_hex(frame->r9);
    serial_write("\n");
    
    serial_write("R10: ");
    serial_write_hex(frame->r10);
    serial_write("  R11: ");
    serial_write_hex(frame->r11);
    serial_write("\n");
    
    serial_write("R12: ");
    serial_write_hex(frame->r12);
    serial_write("  R13: ");
    serial_write_hex(frame->r13);
    serial_write("\n");
    
    serial_write("R14: ");
    serial_write_hex(frame->r14);
    serial_write("  R15: ");
    serial_write_hex(frame->r15);
    serial_write("\n");
    
    serial_write("\n================================\n");
    serial_write("===   SYSTEM HALTED          ===\n");
    serial_write("================================\n");

    if (frame->int_no == 14) {
        uint64_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
    
        uint64_t virt_page = cr2 & ~0xFFF;
        uint64_t phys_page = allocate_page();
    
        if (phys_page == 0) {
            serial_write("ERROR: Cannot allocate page!\n");
            serial_write("System halted.\n");
            for(;;) asm("hlt");
        }
    
        map_page(virt_page, phys_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
        enable_interrupts();
        return;
    }
    
    while (1) {
        asm volatile("hlt");
    }
}

void irq_handler(struct interrupt_frame *frame) {
    // right now we only handle keyboard IRQ and timer IRQ
    uint64_t irq_num = frame->int_no - 32;
    
    if (irq_num == 0) {
        // timer IRQ
        // we can add timer handling code here later
    } else if (irq_num == 1) {
        uint8_t scancode = inb(0x60);
        keyboard_handle_irq(scancode);
    }
    else if (irq_num == 8) {
        rtc_handler();
    }
    else if (irq_num == 12) {
        // mouse IRQ - not implemented yet
    }
    send_eoi(irq_num);
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

    for (int i = 0; i < 32; i++) {
        set_idt_entry(i, isr_stubs[i], 0x8E);
    }
    
    struct idtr idtr;
    idtr.limit = (sizeof(idt_entry_t) * MAX_NUM_IDT_ENTRIES) - 1;
    idtr.base = (uint64_t)&idt;
    
    asm volatile("lidt %0" :: "m"(idtr));
    
    serial_write("IDT loaded!\n");

    init_pic();

    set_idt_entry(32, irq_stub_0, 0x8E);
    set_idt_entry(33, irq_stub_1, 0x8E);
    set_idt_entry(40, irq_stub_8, 0x8E);

    pic_unmask_irq(0);
    pic_unmask_irq(1); 

    keyboard_init();
    

    enable_interrupts();

    serial_write("Interrupts enabled!\n");
}
