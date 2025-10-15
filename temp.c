#include<stdio.h>
#include<stdlib.h>

int main(){
    char *filename = "framedInputData.fram";
    FILE *fptr;
    fptr = fopen(filename, "r");
    if (fptr == NULL){
        printf("file open failed in temp\n");
        return 1;
    }
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET); // same as rewind(f);
    printf("fsize: %ld\n", fsize);
    return 0;
}
