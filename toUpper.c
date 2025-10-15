#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// This program reads from "physicalData.binf" and expects the content to be a
// sequence of bytes represented as characters (like '0' and '1' or ascii text).
// It will convert alphabetic characters to uppercase and then write a new
// framedData.fram where the data part is the uppercase text.

int main(void) {
    FILE *fptr = fopen("physicalData.binf", "r");
    if (!fptr) return 1;
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    char *buf = malloc(fsize + 1);
    if (!buf) { fclose(fptr); return 1; }
    fread(buf, 1, fsize, fptr);
    buf[fsize] = '\0';
    fclose(fptr);

    // Convert alphabetic characters to uppercase in-place
    for (long i = 0; i < fsize; ++i) {
        buf[i] = (char)toupper((unsigned char)buf[i]);
    }

    // Create a new framedData.fram with the uppercase data
    FILE *out = fopen("framedData.fram", "w");
    if (!out) { free(buf); return 1; }
    fprintf(out, "%c%c\n%ld\n%s", 22, 22, fsize, buf);
    fclose(out);
    free(buf);
    return 0;
}
