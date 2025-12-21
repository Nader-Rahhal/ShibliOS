#pragma once
#include <stdint.h>

void ata_wait_busy(void);

void ata_wait_drq(void);

void ata_identify(void);

void ata_read_sector(uint32_t lba, uint8_t *buffer);

void ata_write_sector(uint32_t lba, uint8_t *buffer);