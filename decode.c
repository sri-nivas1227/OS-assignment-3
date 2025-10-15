#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper: convert 8 chars '0'/'1' to a byte (0-255). Returns -1 on error.
static int bits_to_byte(const char *bits) {
    if (!bits) return -1;
    int val = 0;
    for (int i = 0; i < 8; ++i) {
        if (bits[i] == '1') {
            val = (val << 1) | 1;
        } else if (bits[i] == '0') {
            val = (val << 1);
        } else {
            return -1; // invalid character
        }
    }
    return val;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.chck>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    // Read entire file into memory (it's small)
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        perror("fseek");
        return 1;
    }
    long fsize = ftell(f);
    if (fsize < 0) {
        fclose(f);
        perror("ftell");
        return 1;
    }
    rewind(f);

    char *buf = malloc(fsize + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    size_t nread = fread(buf, 1, fsize, f);
    fclose(f);
    buf[nread] = '\0';

    // Expect a stream of '0'/'1' characters. Trim whitespace/newlines.
    char *bits = malloc(nread + 1);
    if (!bits) { free(buf); fprintf(stderr, "malloc failed\n"); return 1; }
    size_t bi = 0;
    for (size_t i = 0; i < nread; ++i) {
        if (buf[i] == '0' || buf[i] == '1') {
            bits[bi++] = buf[i];
        }
    }
    bits[bi] = '\0';
    free(buf);

    // Need at least 16 + 8 = 24 bits for syn syn + length
    if (bi < 24) {
        fprintf(stderr, "file too short: only %zu bits\n", bi);
        free(bits);
        return 1;
    }

    // Parse first two bytes (16 bits) as two sync (SYN) characters
    int syn1 = bits_to_byte(bits);
    int syn2 = bits_to_byte(bits + 8);

    // Parse next 8 bits as length (number of payload bytes)
    int length = bits_to_byte(bits + 16);

    // Ensure we have enough bits for payload
    size_t required_bits = 24 + (size_t)length * 8;
    if (bi < required_bits) {
        fprintf(stderr, "file truncated: expected %zu bits for payload but have %zu\n", required_bits, bi);
        free(bits);
        return 1;
    }

    // Allocate payload string
    char *payload = malloc(length + 1);
    if (!payload) { free(bits); fprintf(stderr, "malloc failed\n"); return 1; }
    for (int i = 0; i < length; ++i) {
        int val = bits_to_byte(bits + 24 + i * 8);
        if (val < 0) val = '?';
        payload[i] = (char)val;
    }
    payload[length] = '\0';

    // Print the first two sync bytes directly as characters (no special-case text).
    char s[3] = { (char)syn1, (char)syn2, '\0' };
    // Write them with printf — they may be non-printable control characters (SYN = 0x16).
    printf("%s\n", s);

    printf("%d\n", length);
    printf("%s\n", payload);

    free(bits);
    free(payload);
    return 0;
}