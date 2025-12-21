#include "str.h"
#include "serial.h"

bool strcmp(const char* c1, const char* c2) {
    if (!c1 || !c2) {
        return false;
    }

    while (*c1 && *c2) {
        if (*c1 != *c2) {
            return false;
        }
        c1++;
        c2++;
    }
    return *c1 == '\0' && *c2 == '\0';
}

size_t strlen(const char* s) {
    size_t len = 0;
    if (!s) return 0;
    while (s[len]) {
        len++;
    }
    return len;
}

bool strcmp_dbg(const char* c1, const char* c2) {
    serial_write("\n[strcmp debug]\n");

    if (!c1 || !c2) {
        serial_write("NULL pointer detected\n");
        return false;
    }

    // Print c1
    serial_write("c1: \"");
    serial_write(c1);
    serial_write("\"\n");
    serial_write("c1 length: ");
    serial_write_dec(strlen(c1));
    serial_write("\n");

    // Print c2
    serial_write("c2: \"");
    serial_write(c2);
    serial_write("\"\n");
    serial_write("c2 length: ");
    serial_write_dec(strlen(c2));
    serial_write("\n");

    // Compare
    while (*c1 && *c2) {
        if (*c1 != *c2) {
            serial_write("Result: NOT EQUAL (char mismatch)\n");
            return false;
        }
        c1++;
        c2++;
    }

    bool equal = (*c1 == '\0' && *c2 == '\0');

    serial_write("Result: ");
    serial_write(equal ? "EQUAL\n" : "NOT EQUAL (length mismatch)\n");

    return equal;
}
