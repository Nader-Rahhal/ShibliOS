#include <stdint.h>

#include "ata.h"
#include "serial.h"
#include "interrupts.h"
#include "terminal.h"
#include "io.h"


void ata_wait_busy(void){
    while (inb(0x1F7) & 0x80);
}

void ata_wait_drq(void) {
    while (!(inb(0x1F7) & 0x08));
}


void ata_identify(void) {

    outb(0x1F6, 0xA0);
    
    for (int i = 0; i < 1000; i++) {
        inb(0x1F7);
    }
    
    outb(0x1F2, 0);  // Sector count
    outb(0x1F3, 0);  // LBA low
    outb(0x1F4, 0);  // LBA mid
    outb(0x1F5, 0);  // LBA high
    
    outb(0x1F7, 0xEC);
    
    uint8_t status = inb(0x1F7);
    if (status == 0) {
        serial_write("Drive does not exist (status = 0)\n");
        return;
    }
    
    serial_write("Drive responded! Status: 0x");
    serial_write_hex(status);
    serial_write("\n");
    
    ata_wait_busy();
    
    uint8_t lba_mid = inb(0x1F4);
    uint8_t lba_hi = inb(0x1F5);
    if (lba_mid != 0 || lba_hi != 0) {
        serial_write("ATAPI device detected, not ATA\n");
        return;
    }

    ata_wait_drq();

    uint16_t data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = inw(0x1F0);
    }
    
    uint32_t sectors = data[60] | ((uint32_t)data[61] << 16);
    serial_write("Sectors: ");
    serial_write_dec(sectors);
    serial_write("\n");

    uint64_t bytes = (uint64_t)sectors * 512;
    uint64_t mb = bytes / (1024 * 1024);
    
    serial_write("Size: ");
    serial_write_dec(mb);
    serial_write(" MB\n");
    serial_write("\n");
}


void ata_read_sector(uint32_t lba, uint8_t *buffer){

    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));

    outb(0x1F2, 0x01);

    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);

    outb(0x1F7, 0x20);

    ata_wait_drq();

    uint16_t *buf = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(0x1F0);
    }
}

void ata_write_sector(uint32_t lba, uint8_t *buffer) {
    ata_wait_busy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));

    outb(0x1F2, 0x01);

    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);

    outb(0x1F7, 0x30);

    ata_wait_drq();

    uint16_t *buf = (uint16_t *)buffer;
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buf[i]);
    }

    outb(0x1F7, 0xE7);
    ata_wait_busy();
}

void read(uint32_t start_sector, uint32_t size, uint8_t *buffer){
    uint32_t total_sectors = size /  512;
    uint32_t offset = 0;

    for (uint32_t sector = start_sector; sector < total_sectors + start_sector; sector++){
        ata_read_sector(sector, buffer + offset);
        offset += 512;
    }
}

void write(uint32_t start_sector, uint32_t size, uint8_t *buffer){
    uint32_t total_sectors = size /  512;
    uint32_t offset = 0;
    for (uint32_t sector = start_sector; sector < total_sectors + start_sector; sector++){
        ata_write_sector(sector, buffer + offset);
        offset += 512;
    }
}
