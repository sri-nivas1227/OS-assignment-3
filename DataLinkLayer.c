#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main(int argc, char *argv[])
{
    if(argc<2){
        printf("not enough arguments");
        
        return ;
    }
    char* data = argv[1];
    //printf("data received: %s\n",data);
    int dataLength = strlen(data);
    FILE *fptr;
    fptr = fopen("framedData.fram", "w");
    if (fptr == NULL)
    {
        //printf("file open failed\n");
        return ;
    }
    fprintf(fptr, "%c%c\n%d\n%s", 22,22, dataLength, data);
    fclose(fptr);    
    //printf("Framed data written to framedData.fram file\n");
    return;
}