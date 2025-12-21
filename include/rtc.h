#pragma once
#include <stdint.h>

#define RTC_COMMAND 0x70
#define RTC_DATA    0x71

void rtc_init(void);

void rtc_handler(void);

void rtc_read_time(uint8_t *hour, uint8_t *minute, uint8_t *second);