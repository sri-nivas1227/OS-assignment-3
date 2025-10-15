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
  }
  else
  {
    char *binaryData;
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
      char buffer[4096];
      ssize_t r;
      size_t total = 0;
      // read in chunks and append to binaryData
      while ((r = read(outpipe[0], buffer, sizeof(buffer))) > 0)
      {
        if (counter == 1 && total == 0)
        {
          binaryData = malloc(r + 1);
          if (!binaryData)
          {
            close(outpipe[0]);
            free(dataToProcess);
            return 1;
          }
          memcpy(binaryData, buffer, r);
          total = r;
          binaryData[total] = '\0';
        }
        else
        {
          size_t old_len = total;
          char *nb = realloc(binaryData, old_len + r + 1);
          if (!nb)
          {
            close(outpipe[0]);
            free(dataToProcess);
            free(binaryData);
            return 1;
          }
          binaryData = nb;
          memcpy(binaryData + old_len, buffer, r);
          total = old_len + r;
          binaryData[total] = '\0';
        }
      }
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
    fprintf(stderr, "Final binary data to write to physicalData.binf: %s\n", binaryData);
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
