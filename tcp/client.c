#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

/* This should be some real ip address. */
#define IP "127.0.0.1"
#define PORT 8181

void func() {
    int s;
    struct sockaddr_in sock;
    char buffer[512];
    char* data;

    data = "HEAD / HTTP/1.0\r\n\r\n";

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("socket() error\n");
        return;
    }

    sock.sin_addr.s_addr = inet_addr(IP);
    sock.sin_port = htons(PORT);
    sock.sin_family = AF_INET;

    if (connect(s, (struct sockaddr*) &sock, sizeof(struct sockaddr_in)) != 0) {
        printf("connect() error\n");
        close(s);
        return;
    }

    write(s, data, strlen(data));
    memset(buffer, 0, 512);
    // read(s, buffer, 511);
    close(s);

    // printf("\n%s\n", buffer);
}

int main(void) {
    for (int i = 0; i < 1000; i++) {
        func();
    }
    return 0;
}