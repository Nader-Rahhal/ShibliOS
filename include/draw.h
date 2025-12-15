#pragma once
#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include <stdbool.h>
#include <font.h>

void DrawPixel(int x, int y, uint32_t color, struct limine_framebuffer *framebuffer) {
    volatile uint32_t *fb_ptr = (uint32_t*)framebuffer->address;
    fb_ptr[y * (framebuffer->pitch / 4) + x] = color;  
}

void DrawChar(int x, int y, char c, uint32_t color, struct limine_framebuffer *framebuffer, void *glyphs, struct psf1_header *hdr) {
    unsigned char *glyph = (unsigned char *)glyphs + ((unsigned char)c * hdr->charsize);
    
    for (int row = 0; row < hdr->charsize; row++) {
        unsigned char line = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                DrawPixel(x + col, y + row, color, framebuffer);
            }
        }
    }
}

void DrawString(int x, int y, const char *str, uint32_t color, struct limine_framebuffer *framebuffer, void *glyphs, struct psf1_header *hdr) {
    int offset = 0;
    while (*str) {
        DrawChar(x + offset, y, *str, color, framebuffer, glyphs, hdr);
        offset += 8;
        str++;
    }
}