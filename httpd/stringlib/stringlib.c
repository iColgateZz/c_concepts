#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

char* allowedMethods[4] = {"get", "post", "put", "delete"};

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

void private__normalizePathDoubleDot(char* newPath, int* j) {
    bool slashSeen = 0;
    while (*j >= 0) {
        char c = newPath[*j];
        if (c == '/' && slashSeen) {
            (*j)++;
            break;
        } else if (c == '/') slashSeen = 1;
        newPath[*j] = 0;
        if (*j > 0) (*j)--;
        else if (*j == 0) break;
    }
}

/*
    Remove ./ and ../
    Remove the front /
    Remove //
    Remove query parameters (for now)
    The / at the end is preserved
*/
void normalizePath(const size_t PATH_LENGTH, char path[PATH_LENGTH]) {
    char newPath[PATH_LENGTH];
    bool isDot = false, isDoubleDot = false, isSlash = false;
    int j = 0, i = 0;

    if (path[0] == '/' && path[1] == 0) return;

    memset(newPath, 0, PATH_LENGTH);

    if (path[0] == '/') {
        i = 1;
        isSlash = 1;
    }

    for (; i < PATH_LENGTH - 1; i++) {
        if (isalnum(path[i])) {
            newPath[j++] = path[i];
            isDot = isDoubleDot = isSlash = 0;
        } else if (path[i] == '/' && isSlash) {
            continue;
        } else if (path[i] == '/' && isDot) {
            isDot = 0;
            isSlash = 1;
            newPath[j--] = 0;
            newPath[j] = 0;
        } else if (path[i] == '/' && isDoubleDot) {
            isDoubleDot = 0;
            isSlash = 1;
            private__normalizePathDoubleDot(newPath, &j);
        } else if (path[i] == '/') {
            isSlash = 1;
            newPath[j++] = '/';
        } else if (path[i] == '.' && isDot) {
            isDot = 0;
            isDoubleDot = 1;
            newPath[j++] = '.';
        } else if (path[i] == '.') {
            isDot = 1;
            isSlash = 0;
            newPath[j++] = '.';
        } else if (path[i] == 0 || path[i] == '?')
            break;
    }

    if (!strncmp("..", newPath, PATH_LENGTH) || !strncmp(".", newPath, PATH_LENGTH)) {
        strncpy(newPath, "/", PATH_LENGTH);
    } else if (isDot && j >= 2 && newPath[j - 2] == '/') {
        newPath[j--] = 0;
        newPath[j] = 0;
    } else if (isDoubleDot && j >= 3 && newPath[j - 3] == '/')
        private__normalizePathDoubleDot(newPath, &j);

    newPath[j] = 0;
    if (!strncmp(newPath, "", PATH_LENGTH))
        strncpy(newPath, "/", PATH_LENGTH);
    strncpy(path, newPath, PATH_LENGTH);
}
