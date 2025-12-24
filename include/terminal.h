#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <limine.h>
#include <font.h>

#define CMD_MAX 256

static struct limine_framebuffer *g_fb = NULL;
static void *g_glyphs = NULL;
static struct psf1_header *g_hdr = NULL;
static uint64_t cursor_x = 10;
static uint64_t cursor_y = 50;
static uint32_t text_color = 0xFFFFFF;
static size_t fb_height = 0;
static size_t fb_width = 0;
static bool auto_prompt = false;
static bool accept_input = false;
static char cmd[CMD_MAX];
static size_t cmd_len = 0;

void terminal_prompt(void);

void terminal_write(const char *str);

void terminal_init(struct limine_framebuffer *fb, void *glyphs, struct psf1_header *hdr);

void terminal_clear(void);

void terminal_set_color(uint32_t color);

void terminal_set_cursor(int x, int y);

void terminal_enable_prompt(bool enable);

void terminal_putchar(char c);

void terminal_putchar_external(char c);

void terminal_draw_hline_single(uint32_t color, uint32_t x, uint32_t y, uint32_t length);

void terminal_draw_vline_single(uint32_t color, uint32_t x, uint32_t y, uint32_t length);

void terminal_draw_hline(uint32_t color, uint32_t x, uint32_t y, uint32_t length, uint32_t thickness);

void terminal_draw_vline(uint32_t color, uint32_t x, uint32_t y, uint32_t length, uint32_t thickness);

void terminal_prompt();

void terminal_write(const char* s);

void terminal_write_hex(uint64_t value);

void terminal_write_dec(uint64_t value);