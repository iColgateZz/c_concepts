#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

/* 
    The string is copied to buf, it is zero-terminated.
    Return 0 if the end is reached.
    Return a pointer to the next valid symbol after 'ch', if there is one.
    if buf is NULL, no copying is performed.
    Modifies the value of the variable pointed to by textSize.
    The new value is the size of the string pointed to by the returned pointer.
*/
char* copyUntilChar(char* text, char* buf, const char ch, int bufSize, int* textSize) {
    char* tmp;
    int i;

    tmp = text;
    i = 0;
    if (buf == NULL) bufSize = INT32_MAX;

    while (*tmp && *tmp != ch && i < bufSize - 1 && i < *textSize) {
        if (buf != NULL)
            buf[i] = tolower(*tmp);
        i++;
        tmp++;
    }
    
    if (buf != NULL)
        buf[i] = 0;

    if (*tmp && i < *textSize) {
        *textSize -= (i + 1);
        return ++tmp;
    } else {
        *textSize -= i;
        return tmp;
    }
}
