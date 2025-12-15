#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SERIAL_PORT 0x3F8

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void serial_init(void) {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}


static inline bool serial_transmit_empty(void) {
    return inb(SERIAL_PORT + 5) & 0x20;
}

static inline void serial_putchar(char c) {
    while (!serial_transmit_empty());
    outb(SERIAL_PORT, c);
}

static inline void serial_write(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}

static inline void serial_write_hex(uint64_t value) {
    const char* hex_chars = "0123456789ABCDEF";
    serial_write("0x");
    
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t digit = (value >> i) & 0xF;
        serial_putchar(hex_chars[digit]);
    }
}
