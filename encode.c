#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encdec.h"

char *encodeChar(char c);
char *encodeInt(int a);

void main(int argc, char *argv[])
{
    printf("\nEncode process\n");
    if (argc < 2)
    {
        printf("no input provided to encode");
        return;
    }
    char *inputData = argv[1];
    printf("\n%d arguments\n", argc);
    printf("Input data to encode: %s\n", inputData);
    // char *dataType = argv[2];
    char *binaryData;
    if (argc == 3 && strncmp(argv[2], "int", 3) == 0)
    {
        printf("data is a integer: %d\n", atoi(inputData));
        int ascii = atoi(inputData);
        char *encodedBinary = encodeInt(ascii);

        binaryData = (char *)malloc((strlen(binaryData) + strlen(encodedBinary)) * sizeof(char));

        strncat(binaryData, encodedBinary, strlen(encodedBinary));
    }
    else
    {
        printf("data is a string\n");

        for (int i = 0; i < strlen(inputData); i++)
        {
            char *encodedBinary = encodeChar(inputData[i]);
            if (i == 0)
            {

                binaryData = (char *)malloc(strlen(encodedBinary) * sizeof(char));
            }
            else
            {
                binaryData = (char *)realloc(binaryData, (strlen(binaryData) + strlen(encodedBinary)) * sizeof(char));
            }
            strncat(binaryData, encodedBinary, strlen(encodedBinary));
        }
        // write binaryData to a temporary file
    }
    FILE *fptr;
    fptr = fopen("tempBinary.binf", "w");
    if (fptr == NULL)
    {
        printf("tempBinary file open failed\n");
        return;
    }
    printf("writing to tempBinary.binf file\n");
    fputs(binaryData, fptr);
    fclose(fptr);
    free(binaryData);
    return;
}

char *encodeChar(char c)
{
    int ascii = (int)c;
    return encodeInt(ascii);
}

char *encodeInt(int a)
{
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