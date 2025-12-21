#include <stdint.h>

#include "pic.h"

static inline void outb_b(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb_b(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// read this https://brokenthorn.com/Resources/OSDevPic.html

void init_pic(void){
    outb_b(MASTER_PIC_CMD, 0x11);
    outb_b(SLAVE_PIC_CMD, 0x11);

    outb_b(MASTER_PIC_DATA, IRQ_0);
    outb_b(SLAVE_PIC_DATA, IRQ_8);

    outb_b(MASTER_PIC_DATA, 0x4);
    outb_b(SLAVE_PIC_DATA, 0x2);

    outb_b(MASTER_PIC_DATA, ICW4_8086);
    outb_b(SLAVE_PIC_DATA, ICW4_8086);

    outb_b(MASTER_PIC_DATA, 0x0);
    outb_b(SLAVE_PIC_DATA, 0x0);
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    
    if (irq < 8) {
        port = MASTER_PIC_DATA;
    } else {
        port = SLAVE_PIC_DATA;
        irq -= 8;
    }
    
    uint8_t value = inb_b(port) & ~(1 << irq);
    outb_b(port, value);
}

void send_eoi(uint8_t irq){
    if (irq >= 8){
        outb_b(SLAVE_PIC_CMD, 0x20);
    }
    outb_b(MASTER_PIC_CMD, 0x20);
}

