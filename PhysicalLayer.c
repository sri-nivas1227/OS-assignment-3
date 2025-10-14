#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *readDataFromPipe(int fd);
void writeDataToPipe(int fd, char *data);


void main(int argc, char *argv[])
{
    FILE *fptr;
    fptr = fopen("framedData.fram", "r");
    if (fptr == NULL)
    {
        printf("framedData file open failed\n");
        fclose(fptr);
        return;
    }
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET); // same as rewind(f);
    char *framedData = (char *)malloc(fsize + 1);
    if (!framedData)
    {
        printf("malloc failed\n");
        fclose(fptr);
        return;
    }
    fread(framedData, 1, fsize, fptr);
    framedData[fsize] = '\0';
    fclose(fptr);
    printf("Framed Data read from framedData.fram file: %s\n", framedData);

    char *binaryData;

    // split the data with the \n delimiter
    // char *token = strtok(framedData, "\n");
    int counter = 0;
    for (char *tok = strtok(framedData, "\n"); tok != NULL; tok = strtok(NULL, "\n"))
    {
        counter++; 
        printf("Got: %s\n", tok);

        pid_t encodePID = fork();
        if (encodePID < 0)
        {
            printf("fork failed\n");
            free(framedData);
            return;
        }
        if (encodePID == 0)
        {
            /*ENCODE PROCESS*/
            printf("\nencode header child process\n");
            if (counter == 2)
            {
                execlp("./encode", "./encode", tok,"int", NULL);
            }
            execlp("./encode", "./encode", tok, NULL);
        }
        
        // wait for encode process to finish
        waitpid(encodePID, NULL, 0); 

        // read from tempBinary.binf written by encode process
        fptr = fopen("tempBinary.binf", "r");
        if (fptr == NULL)
        {
            printf("tempBinary file open failed\n");
            fclose(fptr);
            free(framedData);
            if (binaryData) free(binaryData);
            return;
        }
        fseek(fptr, 0, SEEK_END);
        fsize = ftell(fptr);
        fseek(fptr, 0, SEEK_SET); // same as rewind(f);

        if (counter == 1)
        {
            binaryData = (char *)malloc(fsize + 1);
            if (!binaryData)
            {
                printf("malloc failed\n");
                fclose(fptr);
                free(framedData);
                return;
            }
            fread(binaryData, 1, fsize, fptr);
            binaryData[fsize] = '\0';
        }
        else
        {
            size_t old_len = strlen(binaryData);
            binaryData = (char *)realloc(binaryData, old_len + fsize + 1);
            if (!binaryData)
            {
                printf("realloc failed\n");
                fclose(fptr);
                free(framedData);
                return;
            }
            fread(binaryData + old_len, 1, fsize, fptr);
            binaryData[old_len + fsize] = '\0';
        }
        fclose(fptr);
    }
    free(framedData);
    
    // send binaryData to add parity bits
    int parityPipe[2];
    if (pipe(parityPipe) == -1){
        printf("pipe for parity failed\n");
        free(binaryData);
        return;
    }
    pid_t parityPID = fork();
    if(parityPID<0){
        printf("fork failed\n");
        free(binaryData);
        return;
    }
    if (parityPID == 0)
    {
        /*PARITY PROCESS*/
        printf("\nparity child process\n");
        close(parityPipe[1]); // close write end of the pipe
        char *dataFromParent = readDataFromPipe(parityPipe[0]);
        printf("Data received from parent process: %s\n", dataFromParent);
        close(parityPipe[0]); // close read end of the pipe
        execlp("./addParity", "./addParity", dataFromParent, NULL);
    }
    else if (parityPID > 0)
    {
        /*PARENT PROCESS*/
        printf("\nparent process sending data to parity process\n");
        close(parityPipe[0]); // close read end of the pipe
        writeDataToPipe(parityPipe[1], binaryData);
        close(parityPipe[1]); // close write end of the pipe
        waitpid(parityPID, NULL, 0); // wait for parity process to finish
    }

    // now write binaryData to a file named physicalData.binf
    fptr = fopen("physicalData.binf", "w");
    if (fptr == NULL)
    {
        printf("physicaldata file open failed\n");
        // free(binaryData);
        return;
    }
    fputs(binaryData, fptr);
    fclose(fptr);
    // free(binaryData);
    printf("Binary data written to physicalData.binf file\n");
    // free(binaryData);
    return;
}



char *readDataFromPipe(int fd)
{
    char *data = malloc(4096); // assuming the file won't be larger than 4KB, for dynamic cases malloc will help
    ssize_t total = 0;
    ssize_t n;
    while ((n = read(fd, data + total, 4096 - total - 1)) > 0)
    {
        total += n;
        if (total >= 4096 - 1)
        {
            break; // prevent buffer overflow
        }
    }
    data[total] = '\0';
    return data;
}

void writeDataToPipe(int fd, char *data)
{
    printf("Writing data to pipe: %s\n", data);
    size_t len = strlen(data);
    ssize_t written = 0;
    while (written < (ssize_t)len)
    {
        ssize_t n = write(fd, data + written, len - written);
        if (n <= 0)
            break;
        written += n;
    }
}