#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <font.h>
#include <draw.h>
#include <serial.h>

static struct limine_framebuffer *g_fb = NULL;
static void *g_glyphs = NULL;
static struct psf1_header *g_hdr = NULL;
static int cursor_x = 10;
static int cursor_y = 50;
static uint32_t text_color = 0xFFFFFF;
static size_t fb_height = 0;
static size_t fb_width = 0;
static bool auto_prompt = false;  // Controls whether prompts are automatic

// Forward declaration
void terminal_prompt(void);

void terminal_init(struct limine_framebuffer *fb, void *glyphs, struct psf1_header *hdr) {
    g_fb = fb;
    g_glyphs = glyphs;
    g_hdr = hdr;
    cursor_x = 10;
    cursor_y = 50;
    fb_height = g_fb->height;
    fb_width = g_fb->width;
}

void terminal_clear(void) {
    for (size_t i = 0; i < fb_height * fb_width; i++) {
        ((uint32_t*)g_fb->address)[i] = 0x000000;
    }
    cursor_x = 10;
    cursor_y = 10;
}

void terminal_set_color(uint32_t color) {
    text_color = color;
}

void terminal_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

// Enable or disable automatic prompts on newline
void terminal_enable_prompt(bool enable) {
    auto_prompt = enable;
}

void terminal_putchar(char c) {
    if (!g_fb || !g_glyphs || !g_hdr) {
        return;  // Not initialized
    }
    
    if (c == '\b') {
        if (cursor_x > 10) {
            cursor_x -= 8;
            // Erase character
            for (int y = 0; y < g_hdr->charsize; y++) {
                for (int x = 0; x < 8; x++) {
                    DrawPixel(cursor_x + x, cursor_y + y, 0x000000, g_fb);
                }
            }
        }
    } else if (c == '\n') {
        // Newline
        cursor_x = 10;
        cursor_y += g_hdr->charsize;
        // Simple scroll: wrap to top if at bottom
        if (cursor_y >= g_fb->height - g_hdr->charsize) {
            cursor_y = 10;
        }
        // Only print prompt automatically if auto_prompt is enabled
        if (auto_prompt) {
            terminal_prompt();
        }
    } else if (c == '\t') {
        cursor_x = ((cursor_x / 32) + 1) * 32;
        if (cursor_x >= g_fb->width - 8) {
            cursor_x = 10;
            cursor_y += g_hdr->charsize;
        }
    } else {
        // Regular character
        DrawChar(cursor_x, cursor_y, c, text_color, g_fb, g_glyphs, g_hdr);
        cursor_x += 8;
        // Wrap to next line
        if (cursor_x >= g_fb->width - 8) {
            cursor_x = 10;
            cursor_y += g_hdr->charsize;
            if (cursor_y >= g_fb->height - g_hdr->charsize) {
                cursor_y = 10;
            }
        }
    }
}

void terminal_draw_hline_single(uint32_t color, uint32_t x, uint32_t y, uint32_t length) {
    size_t start_offset = y * fb_width + x;
    for (size_t i = 0; i < length; i++) {
        if (x + i < fb_width) {
            ((uint32_t*)g_fb->address)[start_offset + i] = color;
        }
    }
}

void terminal_draw_vline_single(uint32_t color, uint32_t x, uint32_t y, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        if (y + i < fb_height) {
            size_t offset = (y + i) * fb_width + x;
            ((uint32_t*)g_fb->address)[offset] = color;
        }
    }
}

void terminal_draw_hline(uint32_t color, uint32_t x, uint32_t y, uint32_t length, uint32_t thickness) {
    for (uint32_t t = 0; t < thickness; t++) {
        terminal_draw_hline_single(color, x, y + t, length);
    }
}

void terminal_draw_vline(uint32_t color, uint32_t x, uint32_t y, uint32_t length, uint32_t thickness) {
    for (uint32_t t = 0; t < thickness; t++) {
        terminal_draw_vline_single(color, x + t, y, length);
    }
}

void terminal_prompt(){
    terminal_putchar('>');
    terminal_putchar(' ');
}

void terminal_write(const char *str) {
    while (*str) {
        terminal_putchar(*str++);
    }
}