#ifndef ENCDEC_H_
#define ENCDEC_H_

// Your declarations go here

#include <stddef.h>

// I/O helpers used across multiple modules
char *readDataFromPipe(int fd);
void writeDataToPipe(int fd, char *data);

// Uppercase conversion helper used by toUpper.c
char *uppercase_binary_ascii(const char *bits_in);

// Encode helpers (declared so other modules can call if needed)
char *encodeChar(char c);
char *encodeInt(int a);



#endif // ENCDEC_H_