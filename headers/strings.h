#include "tools.h"

#ifndef STRINGS_H_
#define STRINGS_H_

void removeHtml(const char * src, char * dest);
wchar_t * charToWchar(const char *src);
char * wcharToChar(const wchar_t *src);


#endif