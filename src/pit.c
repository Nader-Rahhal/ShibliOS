#include <stdint.h>

#include "io.h"
#include "pit.h"

void init_pit(void) {
    
    uint8_t command = 0x36;
    outb(CMD_PORT, command);
    
    // Set frequency to 100 Hz (100 interrupts per second)
    // Reload value = 1193182 / 100 = 11931.82 ≈ 11932 (0x2E9C)
    uint16_t divisor = 11932;
    
    outb(DATA_PORT_0, divisor & 0xFF);
    outb(DATA_PORT_0, (divisor >> 8) & 0xFF);
}