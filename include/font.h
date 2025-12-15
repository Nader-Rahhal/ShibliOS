#include <stdbool.h>

struct psf1_header {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
};

bool verify_psf1(struct psf1_header *hdr){
    return hdr->magic[0] == 0x36 && hdr->magic[1] == 0x04;
}

