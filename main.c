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
        fprintf(stderr, "pipe 1 failed\n");
        return 2;
    }
    pid_t pid = fork();
    if (pid == 0)
    {
        /*CHILD PROCESS*/
        // read from pipefd
        close(pipefd[1]); // close write end of the pipe
        char *inputBinaryFile = readDataFromPipe(pipefd[0]);
        close(pipefd[0]); // close read end of the pipe
        printf("\n Producer/Consumer got %s\n", inputBinaryFile);
    }
    else if (pid > 0)
    {
        /*PARENT PROCESS*/
        // User's producer process
        FILE *fptr;
        fptr = fopen("lowercaseData.inpf", "r");
        if (fptr == NULL)
        {
            fprintf(stderr, "Error opening file!\n");
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
        fread(inputData, 1, fsize, fptr);
        inputData[fsize] = '\0';
        fclose(fptr);

        /*Forking Out to call DataLinkLayer*/
        int dataLinkPipe[2];
        if (pipe(dataLinkPipe) == -1)
        {
            fprintf(stderr, "pipe for data link layer failed\n");
            return 2;
        }
        pid_t dataLinkPID = fork();
        if (dataLinkPID < 0)
        {
            printf("fork failed\n");
            return -1;
        }
        if (dataLinkPID == 0)
        {
            /*DATA LINK LAYER CHILD PROCESS*/
            close(dataLinkPipe[0]); // close read end of the pipe
            dup2(dataLinkPipe[1], STDOUT_FILENO);
            close(dataLinkPipe[1]);
            execlp("./DataLinkLayer", "./DataLinkLayer", inputData, NULL);
        }

        close(dataLinkPipe[1]); // close write end of the pipe
        // read from pipe
        char *framedFile = readDataFromPipe(dataLinkPipe[0]);
        close(dataLinkPipe[0]);
        printf("Framed file created: %s\n", framedFile);
        waitpid(dataLinkPID, NULL, 0); // wait for data link layer process to finish

        // Now Data Link Layer has created a frame in a file framedFile
        fprintf(stderr, "\n\n");
        int physicalPipe[2];
        if (pipe(physicalPipe) == -1)
        {
            fprintf(stderr, "\npipe for physical layer failed\n");
            return 2;
        }
        pid_t physicalPID = fork();
        if (physicalPID < 0)
        {
            printf("fork failed\n");
            return -1;
        }
        if (physicalPID == 0)
        {
            /*PHYSICAL LAYER CHILD PROCESS*/
            close(physicalPipe[0]); // close read end of the pipe
            dup2(physicalPipe[1], STDOUT_FILENO);
            close(physicalPipe[1]);
            fprintf(stderr, "\nPhysical layer process PID: %d\n", getpid());
            execlp("./PhysicalLayer", "./PhysicalLayer", framedFile, NULL);
        }

        // wait for physical layer process to finish
        close(physicalPipe[1]); // close write end of the pipe
        char *physicalDataFile = readDataFromPipe(physicalPipe[0]);
        close(physicalPipe[0]);
        printf("Physical layer output file: %s\n", physicalDataFile);
        waitpid(physicalPID, NULL, 0);
        // write this file name to pipefd
        close(pipefd[0]); // close read end of the pipe
        writeDataToPipe(pipefd[1], physicalDataFile);
        close(pipefd[1]); // close write end of the pipe
        waitpid(pid, NULL, 0);
        // // After PhysicalLayer writes physicalData.binf, run toUpper to convert data
        // pid_t upperPID = fork();
        // if (upperPID < 0)
        // {
        //     return -1;
        // }
        // if (upperPID == 0)
        // {
        //     // child: run toUpper
        //     execlp("./toUpper", "./toUpper", NULL);
        // }
        // // wait for to upper process to finish

        // waitpid(upperPID, NULL, 0);

        // // Now deframe using DataLinkLayer --deframe and capture its stdout by using a pipe
        // int deframePipe[2];
        // if (pipe(deframePipe) == -1)
        // {
        //     printf("pipe for deframe failed\n");
        //     return 2;
        // }
        // pid_t deframePID = fork();
        // if (deframePID < 0)
        // {
        //     printf("fork failed\n");
        //     return -1;
        // }
        // if (deframePID == 0)
        // {
        //     // child: run DataLinkLayer --deframe and write to pipe
        //     close(deframePipe[0]); // close read end
        //     // redirect stdout to write end of pipe
        //     dup2(deframePipe[1], STDOUT_FILENO);
        //     close(deframePipe[1]);
        //     execlp("./DataLinkLayer", "./DataLinkLayer", "--deframe", NULL);
        //     _exit(1);
        // }
        // else
        // {
        //     // parent: read from pipe and print the uppercase data
        //     close(deframePipe[1]); // close write end
        //     char buffer[4096];
        //     ssize_t n = read(deframePipe[0], buffer, sizeof(buffer) - 1);
        //     if (n > 0)
        //     {
        //         buffer[n] = '\0';
        //         printf("Final output:\n%s\n", buffer);
        //     }
        //     else
        //     {
        //         printf("No data received from deframe\n");
        //     }
        //     close(deframePipe[0]);
        //     waitpid(deframePID, NULL, 0);
        // }
    }
    else
    {
        // printf("fork failed\n");
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
    // printf("Writing data to pipe: %s\n", data);
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