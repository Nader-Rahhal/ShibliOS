#include <stdbool.h>
#include <stdint.h>

#define MAX_NUM_IDT_ENTRIES = 256

void enable_interrupts(void) {
    asm volatile ("sti");
}

void disable_interrupts(void){
    asm volatile ("cli");
}

bool interrupts_enabled(void) {
    uint64_t flags;
    asm volatile ("pushfq; pop %0" : "=r"(flags));
    return flags & (1 << 9);
}

// idt kept in idtr register
    // use lidt and pass pointer to idt

struct idtr {
    uint16_t limit;    // Size of IDT - 1
    uint64_t base;     // Address of IDT
} __attribute__((packed));

