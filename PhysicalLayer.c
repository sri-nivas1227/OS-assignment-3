#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char *readDataFromPipe(int fd);
void writeDataToPipe(int fd, char *data);

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    fprintf(stderr, "not enough arguments\n");
    return 1;
  }
  char *filename = argv[1];
  fprintf(stderr, "filename length: %d\n", strlen(filename));
  FILE *fptr;
  fptr = fopen(filename, "r");
  if (fptr == NULL)
  {
    printf("file open failed in PhysicalLayer\n");
    return 1;
  }
  fseek(fptr, 0, SEEK_END);
  long fsize = ftell(fptr);
  fseek(fptr, 0, SEEK_SET); // same as rewind(f);
  char *dataToProcess = (char *)malloc(fsize + 1);
  if (!dataToProcess)
  {
    // printf("malloc failed\n");
    fclose(fptr);
    return 1;
  }
  fread(dataToProcess, 1, fsize, fptr);
  dataToProcess[fsize] = '\0';
  fclose(fptr);
  // printf("Framed Data read from framedData.fram file: %s\n", framedData);
  if (argc == 3 && strcmp(argv[2], "--convert") == 0)
  {
    // ignore header and length bytes and check parity of the actual binary data
    // i.e ignore first 3 bytes = 3*8 = 24 bits
    char *data = dataToProcess + 24;
    char *headerData = (char *)malloc(25);
    if (!headerData)
    {
      free(dataToProcess);
      return 1;
    }
    strncpy(headerData, dataToProcess, 24);
    headerData[24] = '\0';
    int dataLen = strlen(data);
    fprintf(stderr, "Data to convert length: %d and data is: %s\n", dataLen, data);
    // write to a temporary file and call checkParity on it
    char *tempFilename = "tempBinaryData.binf";
    FILE *tempfptr = fopen(tempFilename, "w");
    if (tempfptr == NULL)
    {
      free(dataToProcess);
      return 1;
    }
    fputs(data, tempfptr);
    fclose(tempfptr);

    // call checkParity on data
    int outpipe[2];
    if (pipe(outpipe) == -1)
    {
      free(dataToProcess);
      return 1;
    }
    pid_t checkPID = fork();
    if (checkPID < 0)
    {
      close(outpipe[0]);
      close(outpipe[1]);
      free(dataToProcess);
      return 1;
    }
    if (checkPID == 0)
    {
      /*Check Parity Child Process*/
      close(outpipe[0]);
      if (dup2(outpipe[1], STDOUT_FILENO) == -1)
        _exit(1);
      close(outpipe[1]);
      execlp("./checkParity", "./checkParity", tempFilename, NULL);
      _exit(1);
    }
    // parent: read from outpipe[0] and write it to parityCheckedaInputBinary.binf
    close(outpipe[1]);
    char *parityCheckedData = readDataFromPipe(outpipe[0]);
    close(outpipe[0]);
    waitpid(checkPID, NULL, 0);
    free(dataToProcess);
    // delete temp file
    remove(tempFilename);
    // write parityCheckedData to parityCheckedaInputBinary.binf
    char *outFilename = "parityCheckedaInputBinary.binf";
    fptr = fopen(outFilename, "w");
    if (fptr == NULL)
    {
      free(parityCheckedData);
      return 1;
    }
    fprintf(stderr, "Final binary data after parity check to write to parityCheckedaInputBinary.binf: %s", parityCheckedData);
    fputs(parityCheckedData, fptr);
    fclose(fptr);
    free(parityCheckedData);
    // call convert to uppercase and pass the filename parityCheckedaInputBinary.binf to it
    int convertPipe[2];
    if (pipe(convertPipe) == -1)
    {
      return 1;
    }
    pid_t convertPID = fork();
    if (convertPID < 0)
    {
      close(convertPipe[0]);
      close(convertPipe[1]);
      return 1;
    }
    if (convertPID == 0)
    {
      /*Convert Child Process*/
      close(convertPipe[0]);
      if (dup2(convertPipe[1], STDOUT_FILENO) == -1)
        _exit(1);
      close(convertPipe[1]);
      execlp("./toUpper", "./toUpper", outFilename, NULL);
      _exit(1);
    }
    // parent: read from convertPipe[0]
    close(convertPipe[1]);
    char *upperData = readDataFromPipe(convertPipe[0]);
    close(convertPipe[0]);
    waitpid(convertPID, NULL, 0);
    // delete outFilename file
    remove(outFilename);
    // write upperData to uppercaseDeframedBinaryData.done
    char *finalOutFilename = "uppercaseDeframedBinaryData.done";
    fptr = fopen(finalOutFilename, "w");
    if (fptr == NULL)
    {
      free(upperData);
      return 1;
    }
    fprintf(stderr, "Final uppercase binary data to write to uppercaseDeframedBinaryData.done: %s", upperData);
    fputs(upperData, fptr);
    fclose(fptr);
    // write headerData + upperData to framedUppercaseBinary.chck file
    char *framedOutFilename = "framedUppercaseBinary.chck";
    fptr = fopen(framedOutFilename, "w");
    if (fptr == NULL)
    fptr = fopen(framedOutFilename, "w");
    if (fptr == NULL)
    {
      free(upperData);
      free(headerData);
      return 1;
    }
    fprintf(fptr, "%s%s", headerData, upperData);
    fclose(fptr);
    free(upperData);
    free(headerData);

    // call decode on framedUppercaseBinary.chck with --string option
    int decodePipe[2];
    if (pipe(decodePipe) == -1)
    {
      free(upperData);
      return 1;
    }
    pid_t decodePID = fork();
    if (decodePID < 0)
    {
      close(decodePipe[0]);
      close(decodePipe[1]);
      free(upperData);
      return 1;
    }
    if (decodePID == 0)
    {
      /*Decode Child Process*/
      close(decodePipe[0]);
      if (dup2(decodePipe[1], STDOUT_FILENO) == -1)
        _exit(1);
      close(decodePipe[1]);
      execlp("./decode", "./decode", framedOutFilename, "--string", NULL);
      _exit(1);
    }
    // parent: read from decodePipe[0]
    close(decodePipe[1]);
    char *decodedData = readDataFromPipe(decodePipe[0]);
    close(decodePipe[0]);
    waitpid(decodePID, NULL, 0);
    // write decodedData to finalOutputFrame.fram
    char *finalDecodeOutFilename = "finalOutputFrame.fram";
    fptr = fopen(finalDecodeOutFilename, "w");
    if (fptr == NULL)
    {
      free(decodedData);
      return 1;
    }
    fprintf(stderr, "Final decoded data to write to finalOutputFrame.fram: %s", decodedData);
    fputs(decodedData, fptr);
    fclose(fptr);
    free(decodedData);
    // print the final out  put filename to stdout
    printf("%s", finalDecodeOutFilename);
    fflush(stdout);    
    return 0;
  }
  else
  {
    char *binaryData;
    binaryData = (char *)malloc(1);
    if (!binaryData)
    {
      free(dataToProcess);
      return 1;
    }
    binaryData[0] = '\0'; // start with empty string
    int counter = 0;
    for (char *tok = strtok(dataToProcess, "\n"); tok != NULL;
         tok = strtok(NULL, "\n"))
    {
      counter++;
      fprintf(stderr, "Got token (%d): %s\n", counter, tok);

      int outpipe[2];
      if (pipe(outpipe) == -1)
      {
        free(dataToProcess);
        return 1;
      }
      pid_t encodePID = fork();
      if (encodePID < 0)
      {
        close(outpipe[0]);
        close(outpipe[1]);
        free(dataToProcess);
        return 1;
      }
      if (encodePID == 0)
      {
        /*Encode Lowercase Input Child Process*/
        close(outpipe[0]);
        if (dup2(outpipe[1], STDOUT_FILENO) == -1)
          _exit(1);
        close(outpipe[1]);
        if (counter == 2)
        {
          execlp("./encode", "./encode", tok, "--int", NULL);
        }
        else if (counter == 3)
        {
          execlp("./encode", "./encode", tok, "--string", "--parity", NULL);
        }
        else
        {
          execlp("./encode", "./encode", tok, "--string", NULL);
        }
        _exit(1);
      }

      // parent: read from outpipe[0]
      close(outpipe[1]);
      // read all bytes from encode stdout
      char *buffer = readDataFromPipe(outpipe[0]);
      // reallocate binaryData to hold new data and append buffer to it
      size_t oldLen = strlen(binaryData);
      size_t bufLen = strlen(buffer);
      char *newBinaryData = realloc(binaryData, oldLen + bufLen + 1);
      if (!newBinaryData)
      {
        free(binaryData);
        free(buffer);
        close(outpipe[0]);
        waitpid(encodePID, NULL, 0);
        free(dataToProcess);
        return 1;
      }
      binaryData = newBinaryData;
      // append buffer to exisiting binaryData
      strcpy(binaryData + oldLen, buffer);
      // binaryData[oldLen + bufLen] = '\0';
      free(buffer);
      close(outpipe[0]);
      waitpid(encodePID, NULL, 0);
      fprintf(stderr, "After token %d, binaryData: %s\n", counter, binaryData);
    }

    free(dataToProcess);
    // now we have the assembled binaryData with parity bits if any
    // write it to physicalData.binf
    char *outFilename = "physicalData.binf";
    fptr = fopen(outFilename, "w");
    if (fptr == NULL)
    {
      free(binaryData);
      return 1;
    }
    fprintf(stderr, "Final binary data to write to physicalData.binf: %s", binaryData);
    fputs(binaryData, fptr);
    fclose(fptr);
    free(binaryData);
    // printf("Binary data written to physicalData.binf: %s\n", binaryData);
    printf("%s", outFilename);
    fflush(stdout);
    return 0;
  }

  return 0;
}

char *readDataFromPipe(int fd)
{
  char *data = malloc(4096); // assuming the file won't be larger than 4KB, for
                             // dynamic cases malloc will help
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
