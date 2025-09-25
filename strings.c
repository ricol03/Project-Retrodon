#include "headers/tools.h"

void removeHtml(const char * src, char * dest) {
    boolean tag;

    while (*src) {
        if (*src == '<') {
            tag = TRUE;

            if (*++src == 'b' && *++src == 'r') {
                *dest++ = '\n';
                //*dest++ = 'n';
            }
            
        } else if (*src == '>') {
            tag = FALSE;
        } else if (!tag) {
            *dest++ = *src;
        }
        src++;
    }

    *dest = '\0';
}

wchar_t * charToWchar(const char *src) {
    wchar_t * dest;
    MultiByteToWideChar(CP_UTF8, 0, src, -1, dest, sizeof(src));
    return dest;
}

char * wcharToChar(const wchar_t *src) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);

    char *dest = malloc(size_needed);
    if (!dest) return NULL;

    WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, size_needed, NULL, NULL);

    return dest;
}
