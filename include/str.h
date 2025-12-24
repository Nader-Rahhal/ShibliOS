#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool strcmp(const char* c1, const char* c2);

size_t strlen(const char* s);

uint32_t getfirststr(const char* str, char* buffer, uint32_t buffer_size);

bool strcmp_dbg(const char* c1, const char* c2);