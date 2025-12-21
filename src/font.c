#include <stdbool.h>

#include "font.h"

bool verify_psf1(struct psf1_header *hdr){
    return hdr->magic[0] == 0x36 && hdr->magic[1] == 0x04;
}

