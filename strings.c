#include "headers/tools.h"

char * removeHtml(const char * src) {
    char * dest = malloc(sizeof(char) * MAX_STR);
    char * point_start = dest;
    boolean tag;

    while (*src) {
        if (*src == '<') {
            tag = TRUE;

            if (*++src == 'b' && *++src == 'r') {
                *dest++ = '\n';
            }
            
        } else if (*src == '>') {
            tag = FALSE;
        } else if (!tag) {
            *dest++ = *src;
        }
        src++;
    }

    //point *dest back to the start of the string
    dest = point_start;

    return dest;
}

char * removeLetters(char * src) {
    char * final = malloc(MAX_STR);

    char * first = strtok(src, "T");
    char * second = strtok(NULL, ".");

    if (first && second)
        snprintf(final, MAX_STR, "%s %s", first, second);
    else
        snprintf(final, MAX_STR, "%s", "(date unavailable)"); 

    return final;
}

wchar_t * charToWchar(const char *src) {
    wchar_t * dest = malloc(sizeof(wchar_t) * MAX_STR);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, MAX_STR);
    return dest;
}

char * wcharToChar(const wchar_t *src) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);

    char *dest = malloc(size_needed);
    if (!dest) return NULL;

    WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, size_needed, NULL, NULL);

    return dest;
}
