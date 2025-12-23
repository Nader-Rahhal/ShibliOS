#pragma once
#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include "font.h"

void DrawPixel(int x, int y, uint32_t color, struct limine_framebuffer *framebuffer);

void DrawChar(int x, int y, char c, uint32_t color, struct limine_framebuffer *framebuffer, void *glyphs, struct psf1_header *hdr);

void DrawString(int x, int y, const char *str, uint32_t color, struct limine_framebuffer *framebuffer, void *glyphs, struct psf1_header *hdr);