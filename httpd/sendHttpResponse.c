#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char buf[512];
    int n;

    memset(buf, 0, 512);

    sprintf(buf, 
    "HTTP/1.1 %d %s\n"
    "Server: httpd"
    "Content-type: %s\n"
    "\n"
    "%s\n",
    atoi(argv[2]), argv[3], argv[4], argv[5]);

    if (write(atoi(argv[1]), buf, strlen(buf)) < 0) {
        fprintf(stderr, "sendHttpResponse() error\n");
    }
}