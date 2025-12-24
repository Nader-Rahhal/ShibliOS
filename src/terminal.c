#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "limine.h"
#include "font.h"
#include "draw.h"
#include "serial.h"
#include "str.h"
#include "terminal.h"
#include "ext2.h"


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

void terminal_enable_prompt(bool enable) {
    auto_prompt = enable;
}

void terminal_putchar(char c) {
    if (!g_fb || !g_glyphs || !g_hdr) {
        return;
    }

    if (c == '\b') {
        if (cursor_x > 26) {
            if (cmd_len > 0) {
                cmd_len--;
                cmd[cmd_len] = '\0';
            }
            cursor_x -= 8;
            for (int y = 0; y < g_hdr->charsize; y++) {
                for (int x = 0; x < 8; x++) {
                    DrawPixel(cursor_x + x, cursor_y + y, 0x000000, g_fb);
                }
            }
        }
    } else if (c == '\n') {
        cursor_x = 10;
        cursor_y += g_hdr->charsize;
        if (cursor_y >= g_fb->height - g_hdr->charsize) {
            cursor_y = 10;
        }

        cmd[cmd_len] = '\0';
        accept_input = false; 

        char cmd_trimmed[64];
        getfirststr(cmd, cmd_trimmed, sizeof(cmd_trimmed));


        serial_write(cmd_trimmed);
        serial_write("\n");

        if (strcmp(cmd_trimmed, "clear")) {
            terminal_clear();
        }
        else if (strcmp(cmd_trimmed, "dir")) {
            read_directory_entries(2);
        }
        else if (strcmp(cmd_trimmed, "echo")) {
            const char* to_echo = cmd;
            while (*to_echo && (*to_echo == ' ' || *to_echo == '\t')) {
                to_echo++;
            }
            while (*to_echo && *to_echo != ' ' && *to_echo != '\t') {
                to_echo++;
            }
            while (*to_echo && (*to_echo == ' ' || *to_echo == '\t')) {
                to_echo++;
            }
            terminal_write(to_echo);
            terminal_write("\n");
        }

        else if(strcmp(cmd_trimmed, "cat")) {
            print_file(12);
        }

        else if (strcmp(cmd_trimmed, "help")) {
            terminal_write("Available commands:\n");
            terminal_write(" - clear : Clear the terminal screen\n");
            terminal_write(" - dir   : List directory entries of root\n");
            terminal_write(" - echo  : Echo input text\n");
            terminal_write(" - help  : Show this help message\n");
        }
        cmd_len = 0;

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
        if (accept_input && cmd_len < CMD_MAX - 1) {
            cmd[cmd_len++] = c;
            cmd[cmd_len] = '\0';
        }
        DrawChar(cursor_x, cursor_y, c, text_color, g_fb, g_glyphs, g_hdr);
        cursor_x += 8;
        if (cursor_x >= g_fb->width - 8) {
            cursor_x = 10;
            cursor_y += g_hdr->charsize;
            if (cursor_y >= g_fb->height - g_hdr->charsize) {
                cursor_y = 10;
            }
        }
    }
}

void terminal_putchar_external(char c) {
    if (!g_fb || !g_glyphs || !g_hdr) {
        return;
    }

    if (c == '\b') {
        if (cursor_x > 26) {
            cursor_x -= 8;
            for (int y = 0; y < g_hdr->charsize; y++) {
                for (int x = 0; x < 8; x++) {
                    DrawPixel(cursor_x + x, cursor_y + y, 0x000000, g_fb);
                }
            }
        }
    } else if (c == '\n') {
        cursor_x = 10;
        cursor_y += g_hdr->charsize;
        if (cursor_y >= g_fb->height - g_hdr->charsize) {
            cursor_y = 10;
        }

    } else if (c == '\t') {
        cursor_x = ((cursor_x / 32) + 1) * 32;
        if (cursor_x >= g_fb->width - 8) {
            cursor_x = 10;
            cursor_y += g_hdr->charsize;
        }
    } else {
        DrawChar(cursor_x, cursor_y, c, text_color, g_fb, g_glyphs, g_hdr);
        cursor_x += 8;
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

void terminal_prompt() {
    terminal_putchar_external('>');
    terminal_putchar_external(' ');
    accept_input = true;
}

void terminal_write(const char* s) {
    bool prev = accept_input;
    accept_input = false;

    while (*s) {
        terminal_putchar_external(*s++);
    }

    accept_input = prev;
}

void terminal_write_hex(uint64_t value) {
    char hex[17];  // 16 hex digits + null terminator
    hex[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        uint8_t digit = value & 0xF;
        hex[i] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
        value >>= 4;
    }
    
    terminal_write(hex);
}

void terminal_write_dec(uint64_t value) {
    if (value == 0) {
        terminal_write("0");
        return;
    }
    
    char dec[21];  // Max 20 digits for uint64_t + null
    int i = 20;
    dec[i] = '\0';
    
    while (value > 0 && i > 0) {
        i--;
        dec[i] = '0' + (value % 10);
        value /= 10;
    }
    
    terminal_write(&dec[i]);
}