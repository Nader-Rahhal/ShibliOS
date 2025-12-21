#include <stdint.h>
#include <stdbool.h>

#include <serial.h>
#include <terminal.h>

#define SCANCODE_LSHIFT_PRESS   0x2A
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_PRESS   0x36
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_CAPS_LOCK      0x3A

#define KB_BUFFER_SIZE 128

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
static char kb_buffer[KB_BUFFER_SIZE];
static int kb_buffer_read = 0;
static int kb_buffer_write = 0;

void keyboard_init(void);

static void kb_buffer_put(char c);

char keyboard_get_char(void);

bool keyboard_has_char(void);

void keyboard_handle_irq(uint8_t scancode);