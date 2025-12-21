#include <stdint.h>

#include "interrupts.h"
#include "io.h"
#include "rtc.h"

void rtc_init(void) {
    outb(RTC_COMMAND, 0x8B);
    uint8_t prev = inb(RTC_DATA);
    
    outb(RTC_COMMAND, 0x8B);
    outb(RTC_DATA, prev | 0x40);
    
    outb(RTC_COMMAND, 0x8A);
    prev = inb(RTC_DATA);
    outb(RTC_COMMAND, 0x8A);
    outb(RTC_DATA, (prev & 0xF0) | 0x06);
}

void rtc_handler(void) {
    outb(RTC_COMMAND, 0x0C);
    inb(RTC_DATA);
}

void rtc_read_time(uint8_t *hour, uint8_t *minute, uint8_t *second) {
    outb(RTC_COMMAND, 0x04);
    *hour = inb(RTC_DATA);
    
    outb(RTC_COMMAND, 0x02);
    *minute = inb(RTC_DATA);
    
    outb(RTC_COMMAND, 0x00);
    *second = inb(RTC_DATA);
    
    if (!(*hour & 0x80)) {
        *hour = (*hour & 0x0F) + ((*hour / 16) * 10);
        *minute = (*minute & 0x0F) + ((*minute / 16) * 10);
        *second = (*second & 0x0F) + ((*second / 16) * 10);
    }
}
