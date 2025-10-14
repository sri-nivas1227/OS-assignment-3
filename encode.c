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

int main(int argc, char *argv[]) {
  printf("\nEncode process\n");
  if (argc < 2) {
    printf("no input provided to encode");
    return 1;
  }
  char *inputData = argv[1];
  printf("\n%d arguments\n", argc);
  printf("Input data to encode: %s\n", inputData);
  // char *dataType = argv[2];
  char *binaryData;
  if (argc == 3 && strncmp(argv[2], "int", 3) == 0) {
    printf("data is a integer: %d\n", atoi(inputData));
    int ascii = atoi(inputData);
    char *encodedBinary = encodeInt(ascii);

    binaryData = (char *)malloc((strlen(binaryData) + strlen(encodedBinary)) *
                                sizeof(char));

    strncat(binaryData, encodedBinary, strlen(encodedBinary));
  } else {
    printf("data is a string\n");

    for (int i = 0; i < strlen(inputData); i++) {
      char *encodedBinary = encodeChar(inputData[i]);
      if (i == 0) {

        binaryData = (char *)malloc(strlen(encodedBinary) * sizeof(char));
      } else {
        binaryData = (char *)realloc(
            binaryData,
            (strlen(binaryData) + strlen(encodedBinary)) * sizeof(char));
      }
      strncat(binaryData, encodedBinary, strlen(encodedBinary));
    }
    // write binaryData to a temporary file
  }

  // send binaryData to add parity bits if its actual data and not header
  if (argc == 4 && strncmp(argv[3], "parity", 6) == 0) {
    int parityPipe[2];
    if (pipe(parityPipe) == -1) {
      printf("pipe for parity failed\n");
      free(binaryData);
      return 1;
    }
    pid_t parityPID = fork();
    if (parityPID < 0) {
      printf("fork failed\n");
      free(binaryData);
      return 1;
    }
    if (parityPID == 0) {
      /*PARITY PROCESS*/
      printf("\nparity child process\n");
      close(parityPipe[1]); // close write end of the pipe
      char *dataFromParent = readDataFromPipe(parityPipe[0]);
      printf("Data received from parent process: %s\n", dataFromParent);
      close(parityPipe[0]); // close read end of the pipe
      execlp("./addParity", "./addParity", dataFromParent, NULL);
    } else if (parityPID > 0) {
      /*PARENT PROCESS*/
      printf("\nparent process sending data to parity process\n");
      close(parityPipe[0]); // close read end of the pipe
      writeDataToPipe(parityPipe[1], binaryData);
      close(parityPipe[1]);        // close write end of the pipe
      waitpid(parityPID, NULL, 0); // wait for parity process to finish
    }
  } else {

    FILE *fptr;
    fptr = fopen("tempBinary.binf", "w");
    if (fptr == NULL) {
      printf("tempBinary file open failed\n");
      return 1;
    }
    printf("writing to tempBinary.binf file\n");
    fputs(binaryData, fptr);
    fclose(fptr);
    free(binaryData);
  }
  return 0;
}

char *encodeChar(char c) {
  int ascii = (int)c;
  return encodeInt(ascii);
}

char *encodeInt(int a) {
  static char binary[9]; // 8 bits + null terminator
  for (int i = 7; i >= 0; i--) {
    // ascii & i, is a binary and operation
    binary[i] = (a & 1) ? '1' : '0';
    // assign the new value to a
    // by right shifting one bit from existing integer value
    a >>= 1;
  }
  binary[8] = '\0';
  return binary;
}
char *readDataFromPipe(int fd) {
  char *data = malloc(4096); // assuming the file won't be larger than 4KB, for
                             // dynamic cases malloc will help
  ssize_t total = 0;
  ssize_t n;
  while ((n = read(fd, data + total, 4096 - total - 1)) > 0) {
    total += n;
    if (total >= 4096 - 1) {
      break; // prevent buffer overflow
    }
  }
  data[total] = '\0';
  return data;
}

void writeDataToPipe(int fd, char *data) {
  printf("Writing data to pipe: %s\n", data);
  size_t len = strlen(data);
  ssize_t written = 0;
  while (written < (ssize_t)len) {
    ssize_t n = write(fd, data + written, len - written);
    if (n <= 0)
      break;
    written += n;
  }
}