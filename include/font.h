#pragma once
#include <stdbool.h>

struct psf1_header {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char charsize;
};

bool verify_psf1(struct psf1_header *hdr);
