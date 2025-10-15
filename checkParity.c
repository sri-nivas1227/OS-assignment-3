#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// checkParity: verifies odd parity per 8-bit chunk for given binary string
// usage: checkParity <binary_string>
// exit 0 -> parity OK, exit 1 -> parity error or invalid input

int main(int argc, char *argv[]) {
    // Read entire stdin into a buffer
    size_t cap = 4096;
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) return 1;
    ssize_t r;
    while ((r = read(STDIN_FILENO, buf + len, cap - len)) > 0) {
        len += r;
        if (len + 1 >= cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) { free(buf); return 1; }
            buf = nb;
        }
    }
    if (r < 0) { free(buf); return 1; }
    buf[len] = '\0';

    // Trim trailing newlines if any
    while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) { buf[--len] = '\0'; }

    if (len == 0) {
        fprintf(stderr, "checkParity: no input on stdin\n");
        free(buf);
        return 1;
    }

    int blen = (int)len;
    if (blen % 8 != 0) {
        fprintf(stderr, "checkParity: length not multiple of 8 (%d)\n", blen);
        free(buf);
        return 1;
    }
    for (int i = 0; i < blen; i += 8) {
        int count = 0;
        for (int j = 0; j < 8; ++j) {
            if (buf[i+j] == '1') ++count;
            else if (buf[i+j] != '0') {
                fprintf(stderr, "checkParity: invalid char '%c' at pos %d\n", buf[i+j], i+j);
                free(buf);
                return 1;
            }
        }
        if ((count % 2) == 0) {
            fprintf(stderr, "checkParity: parity error at byte %d (count=%d)\n", i/8, count);
            free(buf);
            return 1;
        }
    }
    free(buf);
    return 0;
}
