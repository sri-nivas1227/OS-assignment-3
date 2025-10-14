#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char* toLowerCase(char *s);
char* main(int argc, char*argv[]){
    char* s = argv[1];
    printf("%s\n", toLowerCase(s));
    return toLowerCase(s);

}

char* toLowerCase(char *s){
    char* lowerCaseData = malloc(strlen(s)+1);
    for(int i=0; i<strlen(s); i++){
        if((int)s[i]>=65 && (int)s[i]<=90){
            lowerCaseData[i] = s[i]+32;
        }else{
            lowerCaseData[i]=s[i];
        }
    }
    // lowerCaseData[strlen(s)] = '\0';
    return lowerCaseData;
}