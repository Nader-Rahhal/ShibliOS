#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <draw.h>
#include <interrupts.h>
#include <terminal.h>
#include <paging.h>

__attribute__((used, section(".limine_requests")))
volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST_ID,
    .revision = 0,
    .response = NULL,
    .mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .max_mode = LIMINE_PAGING_MODE_X86_64_4LVL,
    .min_mode = LIMINE_PAGING_MODE_X86_64_4LVL
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_address_request exec_addr_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
    return 0;
}

static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

static struct limine_file *find_module(const char *name) {
    if (!module_request.response) {
        return NULL;
    }
    struct limine_module_response *resp = module_request.response;
    for (uint64_t i = 0; i < resp->module_count; i++) {
        struct limine_file *mod = resp->modules[i];
        if (!mod || !mod->path) {
            continue;
        }
        const char *p = mod->path;
        const char *q = name;
        while (*p && *q && *p == *q) {
            p++;
            q++;
        }
        if (*p == '\0' && *q == '\0') {
            return mod;
        }
    }
    return NULL;
}

void kmain(void) {
    serial_init();
    
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }
    
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    
    if (module_request.response == NULL || module_request.response->module_count < 1) {
        hcf();
    }
    
    void *font_data;
    struct limine_file *font = find_module("/boot/font.psf");
    if (!font) {
        hcf();
    }
    
    font_data = font->address;
    struct psf1_header *hdr = font_data;
    
    if (!verify_psf1(hdr)) {
        hcf();
    }
    
    void *glyphs = (void *)((uintptr_t)font_data + sizeof(struct psf1_header));
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    
    terminal_init(framebuffer, glyphs, hdr);
    terminal_clear();
    
    terminal_write("Hello from ShibliOS!\n");
    terminal_set_color(0x00FF00);
    terminal_write("PSF1 font loaded successfully!\n");
    terminal_set_color(0xFFFFFF);
    
    terminal_draw_hline(0xFFFFFF, 0, 50, 300, 3);
    terminal_draw_vline(0xFFFFFF, 300, 0, 53, 3);
    terminal_enable_prompt(true);
    terminal_set_cursor(10, 75);

    setup_paging();
    pmm_init();
    
    terminal_write("\n=== Testing Page Allocation ===\n");
    uint64_t test_page = allocate_page();
    terminal_write("Allocated page at physical: 0x");
    terminal_write_hex(test_page);
    terminal_write("\n");
    
    pmm_stats();
    
    terminal_write("\n=== Testing Page Mapping ===\n");
    uint64_t test_virt = 0x0000000100000000;
    map_page(test_virt, test_page, PAGE_PRESENT | PAGE_WRITE);
    terminal_write("Mapped virtual 0x");
    terminal_write_hex(test_virt);
    terminal_write(" to physical 0x");
    terminal_write_hex(test_page);
    terminal_write("\n");
    
    uint64_t phys_check = get_physical_address(test_virt);
    terminal_write("Lookup returned: 0x");
    terminal_write_hex(phys_check);
    if (phys_check == test_page) {
        terminal_set_color(0x00FF00);
        terminal_write(" ✓ CORRECT\n");
    } else {
        terminal_set_color(0xFF0000);
        terminal_write(" ✗ WRONG\n");
    }
    terminal_set_color(0xFFFFFF);
    
    terminal_write("\n=== Testing Read/Write Through Mapping ===\n");
    
    uint64_t *virt_ptr = (uint64_t *)test_virt;
    uint64_t test_values[] = {
        0xDEADBEEF12345678,
        0xCAFEBABE87654321,
        0x1234567890ABCDEF,
        0xFEDCBA0987654321
    };
    
    terminal_write("Writing test values...\n");
    for (int i = 0; i < 4; i++) {
        virt_ptr[i] = test_values[i];
        terminal_write("  [");
        terminal_write_dec(i);
        terminal_write("] Wrote: 0x");
        terminal_write_hex(test_values[i]);
        terminal_write("\n");
    }
    
    terminal_write("\nReading back values...\n");
    bool all_match = true;
    for (int i = 0; i < 4; i++) {
        uint64_t read_value = virt_ptr[i];
        terminal_write("  [");
        terminal_write_dec(i);
        terminal_write("] Read:  0x");
        terminal_write_hex(read_value);
        
        if (read_value == test_values[i]) {
            terminal_set_color(0x00FF00);
            terminal_write(" ✓\n");
            terminal_set_color(0xFFFFFF);
        } else {
            terminal_set_color(0xFF0000);
            terminal_write(" ✗ EXPECTED 0x");
            terminal_write_hex(test_values[i]);
            terminal_write("\n");
            terminal_set_color(0xFFFFFF);
            all_match = false;
        }
    }
    
    terminal_write("\nVerifying through physical address (HHDM)...\n");
    uint64_t hhdm = hhdm_request.response->offset;
    uint64_t *phys_ptr = (uint64_t *)(hhdm + test_page);
    
    bool phys_match = true;
    for (int i = 0; i < 4; i++) {
        uint64_t phys_value = phys_ptr[i];
        terminal_write("  [");
        terminal_write_dec(i);
        terminal_write("] Physical read: 0x");
        terminal_write_hex(phys_value);
        
        if (phys_value == test_values[i]) {
            terminal_set_color(0x00FF00);
            terminal_write(" ✓\n");
            terminal_set_color(0xFFFFFF);
        } else {
            terminal_set_color(0xFF0000);
            terminal_write(" ✗\n");
            terminal_set_color(0xFFFFFF);
            phys_match = false;
        }
    }
    
    terminal_write("\n");
    if (all_match && phys_match) {
        terminal_set_color(0x00FF00);
        terminal_write("SUCCESS: Virtual mapping works correctly!\n");
        terminal_write("Both virtual and physical access match!\n");
        terminal_set_color(0xFFFFFF);
    } else {
        terminal_set_color(0xFF0000);
        terminal_write("FAILURE: Some tests failed!\n");
        terminal_set_color(0xFFFFFF);
    }
    
    terminal_set_color(0xFFFFFF);
    terminal_prompt();
    idt_init();
    
    hcf();
}