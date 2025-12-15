#include <stdint.h>

// Scancode to ASCII table (US keyboard layout)
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

// Scancode definitions
#define SCANCODE_LSHIFT_PRESS   0x2A
#define SCANCODE_LSHIFT_RELEASE 0xAA
#define SCANCODE_RSHIFT_PRESS   0x36
#define SCANCODE_RSHIFT_RELEASE 0xB6
#define SCANCODE_BACKSPACE      0x0E
#define SCANCODE_ENTER          0x1C
