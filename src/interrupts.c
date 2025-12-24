#include <stdbool.h>
#include <stdint.h>

#include "serial.h"
#include "pic.h"
#include "draw.h"     
#include "font.h"      
#include "keyboard.h"
#include "paging.h"
#include "rtc.h"
#include "interrupts.h"
#include "io.h"
#include "pit.h"


void enable_interrupts(void) {
    asm volatile ("sti");
}

void disable_interrupts(void) {
    asm volatile ("cli");
}

void set_idt_entry(uint8_t vector, void* isr, uint8_t flags) {
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

        serial_write("Faulting address (CR2): 0x");
        serial_write_hex(cr2);
        serial_write("\n");
    
        uint64_t virt_page = cr2 & ~0xFFF;
        uint64_t phys_page = allocate_page();
    
        if (phys_page == 0) {
            serial_write("ERROR: Cannot allocate page!\n");
            serial_write("System halted.\n");
            for(;;) asm("hlt");
        }

        
    
        map_page(virt_page, phys_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
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

void idt_init(void) {
    
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
    set_idt_entry(46, irq_stub_14, 0x8E);
    set_idt_entry(47, irq_stub_15, 0x8E);

    pic_unmask_irq(0);
    pic_unmask_irq(1); 

    keyboard_init();
    init_pit();

    

    enable_interrupts();

    serial_write("Interrupts enabled!\n");
}
