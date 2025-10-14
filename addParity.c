#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

void main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No data received from parent process\n");
        return;
    }
    char *binaryFrame = argv[1];
    printf("Framed Data received from parent process: %s\n", binaryFrame);
    // get substring binaryFrame[3*8:]
    char *binaryData = strdup(binaryFrame + 24); // skip first 3 bytes (3*8=24 bits)
    if (!binaryData)
    {
        printf("strdup failed\n");
        return;
    }
    printf("Binary Data extracted from framed data: %s\n", binaryData);
    // add parity bits to binaryData for each byte (8 bits)
    // odd parity for each byte, if byte is 01110010, it has even 1s so the new byte will be 11110010
    int len = strlen(binaryData);
    char *parityData = (char *)malloc(len + len / 8 + 1); // +1 for null terminator
    if (!parityData)
    {
        printf("malloc failed\n");
        free(binaryData);
        return;
    }
    parityData[0] = '\0'; // initialize to empty string
    for (int i = 0; i < len; i += 8)
    {
        char byte[9]; // 8 bits + null terminator
        strncpy(byte, binaryData + i, 8);
        byte[8] = '\0';
        int count = 0;
        for (int j = 0; j < 8; j++)
        {
            if (byte[j] == '1')
                count++;    
            
        }
        if (count % 2 == 0)
        {
            // even number of 1s, change first bit to 1
            byte[0] = '1';
        }
        else
        {
            // odd number of 1s, change first bit to 0
            byte[0] = '0';
        }
        strcat(parityData, byte);
        
}