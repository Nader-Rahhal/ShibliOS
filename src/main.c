#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <draw.h>
#include <interrupts.h>
// Set the base revision to 4, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST_ID,
    .revision = 0
};

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;



// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

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

// Halt and catch fire function.
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


// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    if (module_request.response == NULL || module_request.response->module_count < 1) {
        hcf();
    }

    void *font_data;

     struct limine_file *font = find_module("/boot/font.psf");


    if (!font) {
        hcf();; // we are egetting caught here
    }

    font_data = font->address;

    struct psf1_header *hdr = font_data;

    if (!verify_psf1(hdr)) {
        hcf();
    }

    void *glyphs = (void *)((uintptr_t)font_data + sizeof(struct psf1_header));

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Note: we assume the framebuffer model is RGB with 32-bit pixels.

     for (size_t i = 0; i < framebuffer->height * framebuffer->width; i++) {
        ((uint32_t*)framebuffer->address)[i] = 0x000000;
    }

    DrawString(10, 10, "Hello from myOS!", 0xFFFFFF, framebuffer, glyphs, hdr);
    DrawString(10, 30, "PSF1 font loaded successfully!", 0x00FF00, framebuffer, glyphs, hdr);
    
    enable_interrupts();

    
    // We're done, just hang...
    hcf();
}