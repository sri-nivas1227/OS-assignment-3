#include "encdec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char *encodeChar(char c);
char *encodeInt(int a);
char *readDataFromPipe(int fd);
void writeDataToPipe(int fd, char *data);

int main(int argc, char *argv[])
{
  fprintf(stderr, "ENCODE\nargc: %d\n", argc);
  for (int i = 0; i < argc; i++)
  {
    fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
  }
  fprintf(stderr, "\nENCODE\n");
  if (argc < 2)
  {
    fprintf(stderr, "not enough arguments\n");
    return 1;
  }

  char *inputData = argv[1];
  char *binaryData = (char *)malloc(1 * sizeof(char));
  binaryData[0] = '\0';

  if (argc == 3 && strncmp(argv[2], "--int", 5) == 0)
  {
    int integer = atoi(inputData);
    char *encodedBinary = encodeInt(integer);

    binaryData = (char *)malloc((strlen(encodedBinary) + 1) * sizeof(char));
    strncat(binaryData, encodedBinary, strlen(encodedBinary));
    binaryData[strlen(encodedBinary)] = '\0';
    // print binary data to stdout
    printf("%s\n", binaryData);
    fflush(stdout);
    return 0;
  }
  else
  {

    for (int i = 0; i < strlen(inputData); i++)
    {
      fprintf(stderr, "Encoding character: %c\n", inputData[i]);
      char *encodedBinary = encodeChar(inputData[i]);
      if (i == 0)
      {
        binaryData = (char *)malloc((strlen(encodedBinary) + 1) * sizeof(char));
      }
      else
      {
        binaryData = (char *)realloc(
            binaryData,
            (strlen(binaryData) + strlen(encodedBinary) + 1) * sizeof(char));
      }
      strncat(binaryData, encodedBinary, strlen(encodedBinary));
    }
    binaryData[strlen(binaryData)] = '\0';
    fprintf(stderr, "Binary data at string encoding: %s\n", binaryData);

    if (argc == 4 && strncmp(argv[3], "--parity", 8) == 0)
    {
      // write binaryData to a temporary file and call parity program
      char *tempFilename = "beforeParityBinaryData.binf";
      FILE *fptr = fopen(tempFilename, "w");
      if (fptr == NULL)
      {
        free(binaryData);
        return 1;
      }
      fputs(binaryData, fptr);
      fclose(fptr);
      // now call parity program with this file
      int parityPipe[2];
      if (pipe(parityPipe) == -1)
      {
        free(binaryData);
        return 1;
      }
      pid_t parityPID = fork();
      if (parityPID < 0)
      {
        close(parityPipe[0]);
        close(parityPipe[1]);
        free(binaryData);
        return 1;
      }
      if (parityPID == 0)
      {
        /*Parity Bit Child Process*/
        close(parityPipe[0]);
        if (dup2(parityPipe[1], STDOUT_FILENO) == -1)
          _exit(1);
        close(parityPipe[1]);
        execlp("./addParity", "./addParity", tempFilename, NULL);
        _exit(1);
      }

      /*Parent Process*/
      close(parityPipe[1]);
      // free(binaryData);
      binaryData = readDataFromPipe(parityPipe[0]);
      close(parityPipe[0]);
      waitpid(parityPID, NULL, 0);
      printf("%s\n", binaryData);
      fflush(stdout);
      return 0;
    }
    else
    {
      printf("%s\n", binaryData);
      fflush(stdout);
      return 0;
    }
  }
  return 0;
}

char *encodeChar(char c)
{
  int ascii = (int)c;
  fprintf(stderr, "Encoding character: %c\n", c);
  return encodeInt(ascii);
}

char *encodeInt(int a)
{
  fprintf(stderr, "Encoding integer: %d\n", a);
  static char binary[9]; // 8 bits + null terminator
  for (int i = 7; i >= 0; i--)
  {
    binary[i] = (a & 1) ? '1' : '0';
    a >>= 1;
  }
  binary[8] = '\0';
  return binary;
}
char *readDataFromPipe(int fd)
{
  char *data = malloc(4096); // assuming the file won't be larger than 4KB, for
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
  fprintf(stderr, "Data read from pipe: %s\n", data);
  return data;
}

void writeDataToPipe(int fd, char *data)
{
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
