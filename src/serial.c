#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "serial.h"

void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t inw(uint16_t port) {
    uint16_t result;
    asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

void serial_init(void) {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}


bool serial_transmit_empty(void) {
    return inb(SERIAL_PORT + 5) & 0x20;
}

void serial_putchar(char c) {
    while (!serial_transmit_empty());
    outb(SERIAL_PORT, c);
}

void serial_write(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r');
        }
        serial_putchar(*str++);
    }
}

void serial_write_hex(uint64_t value) {
    const char* hex_chars = "0123456789ABCDEF";
    serial_write("0x");
    
    for (int i = 60; i >= 0; i -= 4) {
        uint8_t digit = (value >> i) & 0xF;
        serial_putchar(hex_chars[digit]);
    }
}

void serial_write_dec(uint64_t value) {
    if (value == 0) {
        serial_putchar('0');
        return;
    }
    
    char buffer[20];
    int pos = 0;
    
    while (value > 0) {
        buffer[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    for (int i = pos - 1; i >= 0; i--) {
        serial_putchar(buffer[i]);
    }
}

