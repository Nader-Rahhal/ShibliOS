#include <stdint.h>
#include <stdbool.h>

#include "serial.h"
#include "terminal.h"
#include "keyboard.h"

void keyboard_init(void) {
    shift_pressed = false;
    caps_lock = false;
    kb_buffer_read = 0;
    kb_buffer_write = 0;
}

static void kb_buffer_put(char c) {
    int next = (kb_buffer_write + 1) % KB_BUFFER_SIZE;
    if (next != kb_buffer_read) {
        kb_buffer[kb_buffer_write] = c;
        kb_buffer_write = next;
    }
}

char keyboard_get_char(void) {
    if (kb_buffer_read == kb_buffer_write) {
        return 0;
    }
    
    char c = kb_buffer[kb_buffer_read];
    kb_buffer_read = (kb_buffer_read + 1) % KB_BUFFER_SIZE;
    return c;
}

bool keyboard_has_char(void) {
    return kb_buffer_read != kb_buffer_write;
}

static inline uint8_t inb_t(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void keyboard_handle_irq(uint8_t scancode) {
    
    if (scancode == SCANCODE_LSHIFT_PRESS || scancode == SCANCODE_RSHIFT_PRESS) {
        shift_pressed = true;
        return;
    }
    if (scancode == SCANCODE_LSHIFT_RELEASE || scancode == SCANCODE_RSHIFT_RELEASE) {
        shift_pressed = false;
        return;
    }
    
    if (scancode == SCANCODE_CAPS_LOCK) {
        caps_lock = !caps_lock;
        return;
    }
    
    if (!(scancode & 0x80)) {
        char c = 0;
        
        if (scancode < sizeof(scancode_to_ascii_lower)) {
            bool uppercase = shift_pressed ^ caps_lock;
            
            if (uppercase) {
                c = scancode_to_ascii_upper[scancode];
            } else {
                c = scancode_to_ascii_lower[scancode];
            }
        }
        
        if (c != 0) {
            kb_buffer_put(c);
            terminal_putchar(c);
        }
    }
}