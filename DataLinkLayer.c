#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    if (argc == 3 && strcmp(argv[2], "--deframe") == 0) {
        char *filename = argv[1];
        FILE *fptr = fopen(filename, "r");
        if (fptr == NULL) return 1;
        fseek(fptr, 0, SEEK_END);
        long fsize = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        char *buf = malloc(fsize + 1);
        if (!buf) { fclose(fptr); return 1; }
        fread(buf, 1, fsize, fptr);
        buf[fsize] = '\0';
        fclose(fptr);
        char *saveptr = NULL;
        char *tok = strtok_r(buf, "\n", &saveptr);
        int token_index = 1;
        char *data_token = NULL;
        while (tok != NULL) {
            if (token_index == 3) { data_token = tok; break; }
            tok = strtok_r(NULL, "\n", &saveptr);
            token_index++;
        }
        if (data_token) printf("%s", data_token);
        free(buf);
        return 0;
    }
    else{

        char *data = argv[1];
        int dataLength = strlen(data);
        const char *outFilename = "framedInputData.fram";
        FILE *fptr = fopen(outFilename, "w");
        if (fptr == NULL) return 1;
        fprintf(fptr, "%c%c\n%d\n%s", 22,22, dataLength, data);
        fclose(fptr);
        /* Print the filename we wrote to stdout so a caller (that execs this program)
        can capture the filename. No extra text, only the filename + newline. */
        printf("%s", outFilename);
        fflush(stdout);
        return 0;
    }
    return 0;
}