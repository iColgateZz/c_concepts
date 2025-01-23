#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* This should be some real ip address. */
#define IP "127.0.0.1"

void func(char* path, int port) {
    int s;
    struct sockaddr_in sock;
    char buffer[512];
    char data[64];

    sprintf(data, "GET %s HTTP/1.0\n", path);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("socket() error\n");
        return;
    }

    sock.sin_addr.s_addr = inet_addr(IP);
    sock.sin_port = htons(port);
    sock.sin_family = AF_INET;

    if (connect(s, (struct sockaddr*) &sock, sizeof(struct sockaddr_in)) != 0) {
        printf("connect() error\n");
        close(s);
        return;
    }

    write(s, data, strlen(data));
    memset(buffer, 0, 512);
    read(s, buffer, 511);
    close(s);

    printf("\n%s\n", buffer);
}

int main(int argc, char* argv[]) {
    if (argc <= 3) {
        fprintf(stderr, "Usage: ./%s <int:num> <str:path> <int:port>\n", argv[0]);
        return -1;
    }

    for (int i = 0; i < atoi(argv[1]); i++) {
        func(argv[2], atoi(argv[3]));
    }
    return 0;
}