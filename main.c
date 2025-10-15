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
    if (pid < 0)
    {
        printf("fork failed\n");
        return -1;
    }
    if (pid == 0)
    {
        /*CHILD PROCESS*/
        // read from pipefd
        close(pipefd[1]); // close write end of the pipe
        char *inputBinaryFile = readDataFromPipe(pipefd[0]);
        close(pipefd[0]); // close read end of the pipe
        printf("\n Producer/Consumer got %s\n", inputBinaryFile);
        // call physical layer with --convert to convert the binary file to uppercase version of inputBinaryFile
        int convertPipe[2];
        if (pipe(convertPipe) == -1)
        {
            fprintf(stderr, "pipe for convert failed\n");
            return 2;
        }
        pid_t convertPID = fork();
        if (convertPID < 0)
        {
            printf("fork failed\n");
            return -1;
        }
        if (convertPID == 0)
        {
            /*CONVERT CHILD PROCESS*/
            close(convertPipe[0]); // close read end of the pipe
            if (dup2(convertPipe[1], STDOUT_FILENO) == -1)
                _exit(1);
            close(convertPipe[1]);
            execlp("./PhysicalLayer", "./PhysicalLayer", inputBinaryFile, "--convert", NULL);
        }
        close(convertPipe[1]); // close write end of the pipe
        char *convertedFrameFile = readDataFromPipe(convertPipe[0]);
        close(convertPipe[0]);
        printf("Converted frame file: %s\n", convertedFrameFile);
        waitpid(convertPID, NULL, 0); // wait for convert process to finish

        // call data link layer with --deframe to deframe the convertedFrameFile
        int deframePipe[2];
        if (pipe(deframePipe) == -1)
        {
            fprintf(stderr, "pipe for deframe failed\n");
            return 2;
        }
        pid_t deframePID = fork();
        if (deframePID < 0)
        {
            printf("fork failed\n");
            return -1;
        }
        if (deframePID == 0)
        {
            /*DEFRAME CHILD PROCESS*/
            close(deframePipe[0]); // close read end of the pipe
            dup2(deframePipe[1], STDOUT_FILENO);
            close(deframePipe[1]);
            execlp("./DataLinkLayer", "./DataLinkLayer", convertedFrameFile, "--deframe", NULL);
        }
        close(deframePipe[1]); // close write end of the pipe
        char *deframedOutput = readDataFromPipe(deframePipe[0]);
        close(deframePipe[0]);
        printf("Deframed output: %s\n", deframedOutput);
        // write deframedOutput to uppercaseData.outf
        waitpid(deframePID, NULL, 0); // wait for deframe process to finish
        FILE *outf = fopen("uppercaseData.outf", "w");
        if (outf == NULL)
        {
            fprintf(stderr, "Error opening output file!\n");
            return 1;
        }
        fprintf(outf, "%s", deframedOutput);
        fclose(outf);
        free(deframedOutput);
        free(convertedFrameFile);
        free(inputBinaryFile);
        exit(0);
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
    }

    return 0;
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