#pragma once

#include <stdint.h>

#define MASTER_PIC_CMD 0x20
#define MASTER_PIC_DATA 0x21
#define SLAVE_PIC_CMD 0xA0
#define SLAVE_PIC_DATA 0xA1

#define IRQ_0 0x20
#define IRQ_8 0x28

#define ICW4_8086 0x01


void init_pic(void);

void pic_unmask_irq(uint8_t irq);

void send_eoi(uint8_t irq);