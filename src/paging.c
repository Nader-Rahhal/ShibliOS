#include <stdint.h>
#include <stddef.h>

#include "limine.h"
#include "terminal.h"
#include "paging.h"

void *k_memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}

void *phys_to_virt(uint64_t phys_addr) {
    return (void *)(hhdm_offset + phys_addr);
}

void setup_paging(void) {

    if (!hhdm_request.response) {
        terminal_write("ERROR: HHDM not available\n");
        return;
    }
    hhdm_offset = hhdm_request.response->offset;
    
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    uint64_t pml4_phys = cr3 & ~0xFFF;
    
    pml4 = (struct pml4_entry *)(hhdm_offset + pml4_phys);
}

void pmm_init(void) {
    if (!memmap_request.response) {
        terminal_write("ERROR: Memory map not available\n");
        return;
    }
    
    k_memset(page_bitmap, 0xFF, sizeof(page_bitmap));
    
    struct limine_memmap_response *memmap = memmap_request.response;

    
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        
        if (entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }
        
        uint64_t base_page = entry->base / PAGE_SIZE;
        uint64_t num_pages = entry->length / PAGE_SIZE;
    
        
        for (uint64_t p = 0; p < num_pages; p++) {
            uint64_t page_num = base_page + p;
            
            if (page_num >= BITMAP_SIZE * 8) {
                break;
            }
            
            uint64_t byte_index = page_num / 8;
            uint64_t bit_index = page_num % 8;
            page_bitmap[byte_index] &= ~(1 << bit_index);
            total_pages++;
        }
    }
}

uint64_t allocate_page(void) {
    for (uint64_t i = next_free_page; i < BITMAP_SIZE * 8; i++) {
        uint64_t byte_index = i / 8;
        uint64_t bit_index = i % 8;
        
        if (!(page_bitmap[byte_index] & (1 << bit_index))) {
            page_bitmap[byte_index] |= (1 << bit_index);
            used_pages++;
            next_free_page = i + 1;
            
            uint64_t phys_addr = i * PAGE_SIZE;
            return phys_addr;
        }
    }
    
    terminal_set_color(0xFF0000);
    terminal_write("ERROR: Out of physical memory!\n");
    terminal_set_color(0xFFFFFF);
    return 0;
}

void free_page(uint64_t phys_addr) {
    uint64_t page_num = phys_addr / PAGE_SIZE;
    
    if (page_num >= BITMAP_SIZE * 8) {
        return;
    }
    
    uint64_t byte_index = page_num / 8;
    uint64_t bit_index = page_num % 8;
    
    page_bitmap[byte_index] &= ~(1 << bit_index);
    used_pages--;
    
    if (page_num < next_free_page) {
        next_free_page = page_num;
    }
}

void map_page(uint64_t virtual_addr, uint64_t physical_addr, uint64_t flags) {
    if (!pml4) {
        terminal_write("ERROR: Paging not initialized\n");
        return;
    }
    
    uint64_t pml4_i = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdpt_i = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_i = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_i = (virtual_addr >> 12) & 0x1FF;
    
    struct pml4_entry *pml4e = &pml4[pml4_i];
    if (!pml4e->present) {
        uint64_t new_pdpt_phys = allocate_page();
        if (!new_pdpt_phys) return;
        
        struct pdpt_entry *new_pdpt = (struct pdpt_entry *)phys_to_virt(new_pdpt_phys);
        k_memset(new_pdpt, 0, PAGE_SIZE);
        
        pml4e->present = 1;
        pml4e->rw = 1;
        pml4e->user = (flags & PAGE_USER) ? 1 : 0;
        pml4e->address = new_pdpt_phys >> 12;
    }
    
    uint64_t pdpt_phys = pml4e->address << 12;
    struct pdpt_entry *pdpt = (struct pdpt_entry *)phys_to_virt(pdpt_phys);
    struct pdpt_entry *pdpte = &pdpt[pdpt_i];
    
    if (!pdpte->present) {
        uint64_t new_pd_phys = allocate_page();
        if (!new_pd_phys) return;
        
        struct pd_entry *new_pd = (struct pd_entry *)phys_to_virt(new_pd_phys);
        k_memset(new_pd, 0, PAGE_SIZE);
        
        pdpte->present = 1;
        pdpte->rw = 1;
        pdpte->user = (flags & PAGE_USER) ? 1 : 0;
        pdpte->page_size = 0;
        pdpte->address = new_pd_phys >> 12;
    }
    
    uint64_t pd_phys = pdpte->address << 12;
    struct pd_entry *pd = (struct pd_entry *)phys_to_virt(pd_phys);
    struct pd_entry *pde = &pd[pd_i];
    
    if (!pde->present) {
        uint64_t new_pt_phys = allocate_page();
        if (!new_pt_phys) return;
        
        struct pt_entry *new_pt = (struct pt_entry *)phys_to_virt(new_pt_phys);
        k_memset(new_pt, 0, PAGE_SIZE);
        
        pde->present = 1;
        pde->rw = 1;
        pde->user = (flags & PAGE_USER) ? 1 : 0;
        pde->page_size = 0;
        pde->address = new_pt_phys >> 12;
    }
    
    uint64_t pt_phys = pde->address << 12;
    struct pt_entry *pt = (struct pt_entry *)phys_to_virt(pt_phys);
    struct pt_entry *pte = &pt[pt_i];
    
    pte->present = 1;
    pte->rw = (flags & PAGE_WRITE) ? 1 : 0;
    pte->user = (flags & PAGE_USER) ? 1 : 0;
    pte->address = physical_addr >> 12;
    
    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

void unmap_page(uint64_t virtual_addr) {
    if (!pml4) return;
    
    uint64_t pml4_i = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdpt_i = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_i = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_i = (virtual_addr >> 12) & 0x1FF;
    
    struct pml4_entry *pml4e = &pml4[pml4_i];
    if (!pml4e->present) return;
    
    struct pdpt_entry *pdpt = (struct pdpt_entry *)phys_to_virt(pml4e->address << 12);
    struct pdpt_entry *pdpte = &pdpt[pdpt_i];
    if (!pdpte->present || pdpte->page_size) return;
    
    struct pd_entry *pd = (struct pd_entry *)phys_to_virt(pdpte->address << 12);
    struct pd_entry *pde = &pd[pd_i];
    if (!pde->present || pde->page_size) return;
    
    struct pt_entry *pt = (struct pt_entry *)phys_to_virt(pde->address << 12);
    struct pt_entry *pte = &pt[pt_i];
    
    pte->present = 0;
    pte->address = 0;
    
    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

uint64_t get_physical_address(uint64_t virtual_addr) {
    if (!pml4) return 0;
    
    uint64_t pml4_i = (virtual_addr >> 39) & 0x1FF;
    uint64_t pdpt_i = (virtual_addr >> 30) & 0x1FF;
    uint64_t pd_i = (virtual_addr >> 21) & 0x1FF;
    uint64_t pt_i = (virtual_addr >> 12) & 0x1FF;
    uint64_t offset = virtual_addr & 0xFFF;
    
    struct pml4_entry *pml4e = &pml4[pml4_i];
    if (!pml4e->present) return 0;
    
    struct pdpt_entry *pdpt = (struct pdpt_entry *)phys_to_virt(pml4e->address << 12);
    struct pdpt_entry *pdpte = &pdpt[pdpt_i];
    if (!pdpte->present) return 0;
    
    if (pdpte->page_size) {
        return (pdpte->address << 12) + (virtual_addr & 0x3FFFFFFF);
    }
    
    struct pd_entry *pd = (struct pd_entry *)phys_to_virt(pdpte->address << 12);
    struct pd_entry *pde = &pd[pd_i];
    if (!pde->present) return 0;
    
    if (pde->page_size) {
        return (pde->address << 12) + (virtual_addr & 0x1FFFFF);
    }
    
    struct pt_entry *pt = (struct pt_entry *)phys_to_virt(pde->address << 12);
    struct pt_entry *pte = &pt[pt_i];
    if (!pte->present) return 0;
    
    return (pte->address << 12) + offset;
}

void pmm_stats(void) {
    terminal_write("\n=== Memory Statistics ===\n");
    terminal_write("Total pages: ");
    terminal_write_dec(total_pages);
    terminal_write(" (");
    terminal_write_dec(total_pages * 4);
    terminal_write(" KB)\n");
    
    terminal_write("Used pages: ");
    terminal_write_dec(used_pages);
    terminal_write(" (");
    terminal_write_dec(used_pages * 4);
    terminal_write(" KB)\n");
    
    terminal_write("Free pages: ");
    terminal_write_dec(total_pages - used_pages);
    terminal_write(" (");
    terminal_write_dec((total_pages - used_pages) * 4);
    terminal_write(" KB)\n");
}