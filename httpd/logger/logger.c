#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_RESET_ALL          "\x1b[0m"

void loggerInfo(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
}

void loggerAttention(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    printf(ANSI_COLOR_GREEN);
    vprintf(msg, ap);
    printf(ANSI_RESET_ALL);
    va_end(ap);
}

void loggerWarning(char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    printf(ANSI_COLOR_YELLOW);
    vprintf(msg, ap);
    printf(ANSI_RESET_ALL);
    va_end(ap);
}

void loggerError(FILE* f, char* msg, ...) {
    va_list ap;

    va_start(ap, msg);
    fprintf(f, ANSI_COLOR_RED);
    vfprintf(f, msg, ap);
    fprintf(f, ANSI_RESET_ALL);
    va_end(ap);
}