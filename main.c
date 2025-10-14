#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "encdec.h"

char *readDataFromPipe(int fd);
void writeDataToPipe(int fd, char *data);

int main()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        printf("pipe 1 failed");
        return 2;
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        /*CHILD PROCESS*/
    }
    else if (pid > 0)
    {
        /*PARENT PROCESS*/
        // User's producer process
        FILE *fptr;
        fptr = fopen("lowercaseData.inpf", "r");
        if (fptr == NULL)
        {
            printf("Error opening file!\n");
            return 1;
        }
        fseek(fptr, 0, SEEK_END);
        long fsize = ftell(fptr);
        fseek(fptr, 0, SEEK_SET); // same as rewind(f);
        char *inputData = (char *)malloc(fsize + 1);
        if (!inputData)
        {
            printf("malloc failed\n");
            return 1;
        }
        size_t readBytes = fread(inputData, 1, fsize, fptr);
        (void)readBytes; // silence unused warning if any
        inputData[fsize] = '\0';
        fclose(fptr);
        
        
        int dataLinkPipe[2];
        if (pipe(dataLinkPipe) == -1)
        {
            printf("pipe for data link layer failed\n");
            return 2;
        }
        pid_t dataLinkPID = fork();
        if (dataLinkPID == 0)
        {
            /*DATA LINK LAYER PROCESS*/
            printf("Data link layer process PID: %d\n", getpid());
            close(dataLinkPipe[1]); // close write end of the pipe
            char *dataFromParent = readDataFromPipe(dataLinkPipe[0]);
            execlp("./DataLinkLayer", "./DataLinkLayer", dataFromParent, NULL);
        }
        else if (dataLinkPID > 0)
        {
            /*PARENT PROCESS*/
            printf("Parent process PID: %d\n", getpid());
            close(dataLinkPipe[0]); // close read end of the pipe
            writeDataToPipe(dataLinkPipe[1], inputData);
            close(dataLinkPipe[1]); // close write end of the pipe
            waitpid(dataLinkPID, NULL, 0); // wait for data link layer process to finish

        }
        else
        {
            printf("fork failed\n");
            return -1;
        }

        printf("\n\n");
        int physicalPipe[2];
        pid_t physicalPID = fork();
        if (pipe(physicalPipe) == -1)
        {
            printf("\npipe for physical layer failed\n");
            return 2;
        }
        if (physicalPID == 0)
        {
            /*PHYSICAL LAYER PROCESS*/
            printf("\nPhysical layer process PID: %d\n", getpid());
            close(physicalPipe[1]); // close write end of the pipe
            char *dataFromParent = readDataFromPipe(physicalPipe[0]);
            execlp("./PhysicalLayer", "./PhysicalLayer", NULL);
        }
        else if (physicalPID > 0)
        {
            /*PARENT PROCESS*/
            waitpid(physicalPID, NULL, 0); // wait for physical layer process to finish
            printf("\ndone encoding\n");
            
        }
        else
        {
            printf("fork failed\n");
            return -1;
        }
    }
    else
    {
        printf("fork failed\n");
        return -1;
    }
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