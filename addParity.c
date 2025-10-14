#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[])
{
    printf("ADD PARITY\n");
    
    if (argc < 2)
    {
        printf("No data received from parent process\n");
        return 1;
    }
    char *binaryData = argv[1];
    if (!binaryData)
    {
        printf("strdup failed\n");
        return 1;
    }

    int len = strlen(binaryData);
    char *parityData = (char *)malloc((len+1)*sizeof(char));
    if (!parityData)
    {
        printf("malloc failed\n");
        free(binaryData);
        return 1;
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
            // even number of 1s, change first bit 
            byte[0] = byte[0] == '1' ? '0' : '1';
        }
        strcat(parityData, byte);
        printf("Byte processed: %s\n", byte);
        printf("Current Parity Data: %s\n", parityData);

    }
    
    //printf("Parity Data: %s\n", parityData);
    
    // Write parity data to file
    FILE *fptr;
    fptr = fopen("tempBinary.binf", "w");
    if (fptr == NULL)
    {
        free(binaryData);
        free(parityData);
        return 1;
    }
    //printf("Writing parity data to tempBinary.binf file\n");
    printf("writing %s to tempBinary.binf file\n", parityData);
    fputs(parityData, fptr);
    fclose(fptr);
    
    // Clean up memory
    free(binaryData);
    free(parityData);
    
    return 0;
}