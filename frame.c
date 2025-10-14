#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char* encodeInt(int a);

void main(){
    int header = 22;
    char* headerBinary = encodeInt(header);
    FILE *fptr;
    fptr = fopen("binaryToBeFramed.binf","r");
    // get the file size
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);  // same as rewind(f);
    char *binaryData = (char *)malloc(fsize + 1);
    fread(binaryData, 1, fsize, fptr);
    fclose(fptr);
    binaryData[fsize] = 0;
    // print out binary data
    printf("Binary data from file: %s\n", binaryData);
    int dataLength = strlen(binaryData);
    printf("Length of binary data from file: %d\n", dataLength);
    char* dataLengthBinary = encodeInt(dataLength);
    // create framed data
    char* framedData = (char *)malloc(strlen(headerBinary)*2 + strlen(dataLengthBinary) + strlen(binaryData) + 1);
    framedData[0] = '\0';
    strncat(framedData, headerBinary, strlen(headerBinary));
    strncat(framedData, headerBinary, strlen(headerBinary));
    strncat(framedData, dataLengthBinary, strlen(dataLengthBinary));
    strncat(framedData, binaryData, strlen(binaryData));
    printf("Framed Data: %s\n", framedData);
    // write framed data to a file
    FILE *fptr2;
    fptr2 = fopen("framedBinaryData.binf","w");
    fputs(framedData, fptr2);
    fclose(fptr2);
    return;
    
}

char* encodeInt(int a){
    static char binary[9]; // 8 bits + null terminator
    for (int i = 7; i >= 0; i--)
    {
        // ascii & i, is a binary and operation
        binary[i] = (a & 1) ? '1' : '0';
        // assign the new value to a
        // by right shifting one bit from existing integer value
        a >>= 1;
    }
    binary[8] = '\0';
    return binary;
}