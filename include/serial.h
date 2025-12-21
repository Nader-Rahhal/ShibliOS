#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SERIAL_PORT 0x3F8

void serial_init(void);

bool serial_transmit_empty(void);

void serial_putchar(char c);

void serial_write(const char* str);

void serial_write_hex(uint64_t value);

void serial_write_dec(uint64_t value);
