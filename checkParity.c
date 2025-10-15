#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// checkParity: verifies odd parity per 8-bit chunk for given binary string
// usage: checkParity <binary_string>
// exit 0 -> parity OK, exit 1 -> parity error or invalid input

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <binary_string>\n", argv[0]);
        return 1;
    }
    char *binaryDataFile = argv[1];
    FILE *fptr = fopen(binaryDataFile, "r");
    if (fptr == NULL) {
        fprintf(stderr, "File open failed in checkParity\n");
        return 1;
    }
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET); // same as rewind(f);
    char *binaryData = (char *)malloc(fsize + 1);
    if (!binaryData) {
        fclose(fptr);
        return 1; 
    }
    fread(binaryData, 1, fsize, fptr);
    binaryData[fsize] = '\0';
    fclose(fptr);



    int len = strlen(binaryData);
    if (len % 8 != 0) {
        fprintf(stderr, "Invalid binary data length, not multiple of 8\n");
        free(binaryData);
        return 1;
    }
    // check parity for each 8-bit chunk and make first bit to 0 if it's 1 and if no.of 1s is odd
    for (int i = 0; i < len; i += 8) {
        int countOnes = 0;
        for (int j = 0; j < 8; j++) {
            if (binaryData[i + j] == '1') {
                countOnes++;
            } else if (binaryData[i + j] != '0') {
                fprintf(stderr, "Invalid character in binary data\n");
                free(binaryData);
                return 1;
            }
        }
        if (countOnes % 2 == 0) {
            // parity error
            fprintf(stderr, "Parity error in byte starting at index %d\n", i);
            free(binaryData);
            return 1;
        }else{
            // parity OK, set first bit to 0 if it's 1
            if (binaryData[i] == '1') {
                binaryData[i] = '0';
            }
        }
    }
    // write the corrected binary data to stdout
    printf("%s", binaryData);
    free(binaryData);
    return 0;
}
