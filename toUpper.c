#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// This program reads from "physicalData.binf" and expects the content to be a
// sequence of bytes represented as characters (like '0' and '1' or ascii text).
// It will convert alphabetic characters to uppercase and then write a new
// framedData.fram where the data part is the uppercase text.
char *uppercase_binary_ascii(const char *bits_in) ;
int main(int argc, char *argv[]) {
    // Read the content of file from argv[1]
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <inputfile>\n", argv[0]);
        return 1;
    }
    char *inputFile = argv[1];
    FILE *fptr = fopen(inputFile, "r");
    if (!fptr) return 1;
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    char *buf = malloc(fsize + 1);
    if (!buf) { fclose(fptr); return 1; }
    fread(buf, 1, fsize, fptr);
    buf[fsize] = '\0';
    fclose(fptr);

    // convert the lowercase binary data to uppercase binary data
    char *upper = uppercase_binary_ascii(buf);
    if (!upper) { free(buf); return 1; }
    free(buf);
    
    // write upper to stdout
    printf("%s", upper);
    free(upper);
    // free(buf);
    fflush(stdout);
    

    
    return 0;
}
static int is01(char c) { return c == '0' || c == '1'; }


char *uppercase_binary_ascii(const char *bits_in) {
    if (!bits_in) return NULL;

    // 1) Count only the data bits (ignore whitespace)
    size_t bitcount = 0;
    for (const char *p = bits_in; *p; ++p) {
        if (is01(*p)) bitcount++;
        else if (!isspace((unsigned char)*p)) return NULL; // invalid char
    }
    if (bitcount == 0 || (bitcount % 8) != 0) return NULL;

    // 2) Allocate output: same number of bits + NUL
    char *out = (char *)malloc(bitcount + 1);
    if (!out) return NULL;

    // 3) Decode → uppercase → encode
    size_t out_i = 0;
    unsigned val = 0, acc = 0; // acc counts bits collected for current byte
    for (const char *p = bits_in; *p; ++p) {
        if (!is01(*p)) continue;       // skip whitespace
        val = (val << 1) | (*p - '0'); // accumulate bit
        if (++acc == 8) {
            unsigned char ch = (unsigned char)val;
            if (ch >= 'a' && ch <= 'z') ch = (unsigned char)toupper(ch);

            // write ch back as 8 bits (MSB→LSB)
            for (int b = 7; b >= 0; --b) {
                out[out_i++] = ((ch >> b) & 1) ? '1' : '0';
            }
            // reset for next byte
            val = 0;
            acc = 0;
        }
    }
    out[out_i] = '\0';
    return out;
}
