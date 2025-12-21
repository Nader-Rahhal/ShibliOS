#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#define PAGE_SIZE 4096
#define ENTRIES_PER_TABLE 512
#define BITMAP_SIZE 32768
#define PAGE_PRESENT  0x1
#define PAGE_WRITE    0x2
#define PAGE_USER     0x4

extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_memmap_request memmap_request;

void *k_memset(void *s, int c, size_t n);

struct pml4_entry {
    uint64_t present:1;
    uint64_t rw:1;
    uint64_t user:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t accessed:1;
    uint64_t ignored1:1;
    uint64_t reserved1:1;
    uint64_t ignored2:1;
    uint64_t available:3;
    uint64_t address:40;
    uint64_t reserved2:11;
    uint64_t xd:1;
} __attribute__((packed));

struct pdpt_entry {
    uint64_t present:1;
    uint64_t rw:1;
    uint64_t user:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t accessed:1;
    uint64_t ignored1:1;
    uint64_t page_size:1;
    uint64_t ignored2:1;
    uint64_t available:3;
    uint64_t address:40;
    uint64_t reserved:11;
    uint64_t xd:1;
} __attribute__((packed));

struct pd_entry {
    uint64_t present:1;
    uint64_t rw:1;
    uint64_t user:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t accessed:1;
    uint64_t ignored1:1;
    uint64_t page_size:1;
    uint64_t ignored2:1;
    uint64_t available:3;
    uint64_t address:40;
    uint64_t reserved:11;
    uint64_t xd:1;
} __attribute__((packed));

struct pt_entry {
    uint64_t present:1;
    uint64_t rw:1;
    uint64_t user:1;
    uint64_t pwt:1;
    uint64_t pcd:1;
    uint64_t accessed:1;
    uint64_t dirty:1;
    uint64_t pat:1;
    uint64_t global:1;
    uint64_t available:3;
    uint64_t address:40;
    uint64_t reserved:11;
    uint64_t xd:1;
} __attribute__((packed));

static struct pml4_entry *pml4 = NULL;
static uint64_t hhdm_offset = 0;

static uint8_t page_bitmap[BITMAP_SIZE];
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;
static uint64_t next_free_page = 0;

void *phys_to_virt(uint64_t phys_addr);

void setup_paging(void);

void pmm_init(void);

uint64_t allocate_page(void);

void free_page(uint64_t phys_addr);

void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags);

void unmap_page(uint64_t virtual_addr);

uint64_t get_physical_address(uint64_t virtual_addr);

void pmm_stats(void);