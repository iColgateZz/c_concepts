/* trl - timed read line */
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

char *trl(int timeout) {
    static char buffer[512]; // static saves the info even after the function returns.
    fd_set rfds; // read filedescriptors
    struct timeval tv;
    int ret;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    ret = select(1, &rfds, 0, 0, &tv);
    if (ret && FD_ISSET(0, &rfds)) {
        memset(buffer, 0, 512);
        ret = read(0, buffer, 511);
        if (ret < 1) {
            return 0;
        }
        ret--;
        buffer[ret] = 0; // oveerride \n with 0
        return buffer;
    } else {
        return 0;
    }
}

int main(void) {
    char *name;

    printf("Name: \n ");
    name = trl(5);
    if (name)
        printf("Hello, %s\n", name);
    else
        printf("no name\n");

    return 0;
}