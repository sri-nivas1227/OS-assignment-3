#include <stdio.h>
#include "encdec.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <binaryDataFile>\n", argv[0]);
        return 1;
    }
    char *inputFile = argv[1];
    FILE *fptr = fopen(inputFile, "r");
    if (fptr == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", inputFile);
        return 1;
    }
    fseek(fptr, 0, SEEK_END);
    long len = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    char *buf = (char *)malloc(len + 1);
    if (!buf)
    {
        fclose(fptr);
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    fread(buf, 1, len, fptr);
    buf[len] = '\0';
    fclose(fptr);

    int ilen = (int)len;
    char *parityData = (char *)malloc(ilen + 1);
    if (!parityData)
    {
        free(buf);
        return 1;
    }
    parityData[0] = '\0';
    for (int i = 0; i < ilen; i += 8)
    {
        char byte[9];
        int tocopy = (i + 8 <= ilen) ? 8 : (ilen - i);
        strncpy(byte, buf + i, tocopy);
        if (tocopy < 8)
        {
            // pad with zeros if incomplete
            for (int p = tocopy; p < 8; ++p)
                byte[p] = '0';
        }
        byte[8] = '\0';
        int count = 0;
        for (int j = 0; j < 8; j++)
        {
            if (byte[j] == '1')
                count++;
        }
        if (count % 2 == 0)
        {
            byte[0] = (byte[0] == '1') ? '0' : '1';
        }
        strcat(parityData, byte);
        fprintf(stderr, "Byte processed: %s\n", byte);
    }
    // write parityData to stdout
    fprintf(stderr, "Final parity data: %s\n", parityData);
    printf("%s",parityData);
    fflush(stdout);
    free(buf);
    free(parityData);
    return 0;
}