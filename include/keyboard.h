#include <stdint.h>
#include <stdbool.h>

#include <serial.h>
#include <terminal.h>

#define SCANCODE_LSHIFT_PRESS   0x2A
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_PRESS   0x36
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_CAPS_LOCK      0x3A

static const char scancode_to_ascii_lower[] = {
    0,   0,   '1', '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',  '=',  '\b',
    '\t','q', 'w', 'e', 'r',  't', 'y', 'u', 'i', 'o', 'p', '[', ']',  '\n',
    0,   'a', 's', 'd', 'f',  'g', 'h', 'j', 'k', 'l', ';', '\'','`',
    0,   '\\','z', 'x', 'c',  'v', 'b', 'n', 'm', ',', '.', '/',  0,
    '*', 0,   ' '
};

static const char scancode_to_ascii_upper[] = {
    0,   0,   '!', '@', '#',  '$', '%', '^', '&', '*', '(', ')', '_',  '+',  '\b',
    '\t','Q', 'W', 'E', 'R',  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',  '\n',
    0,   'A', 'S', 'D', 'F',  'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C',  'V', 'B', 'N', 'M', '<', '>', '?',  0,
    '*', 0,   ' '
};

static bool shift_pressed = false;
static bool caps_lock = false;

#define KB_BUFFER_SIZE 128
static char kb_buffer[KB_BUFFER_SIZE];
static int kb_buffer_read = 0;
static int kb_buffer_write = 0;

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