#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8181

int main(void) {
    int s, c;
    socklen_t addrlen;
    struct sockaddr_in srv;
    char buf[512];
    char* data;

    addrlen = 0;
    memset(&srv, 0, sizeof(srv));

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("socket() error\n");
        return -1;
    }

    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = 0;
    srv.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr*) &srv, sizeof(srv)) != 0) {
        printf("bind() error\n");
        close(s);
        return -1;
    }

    if (listen(s, 5) != 0) {
        printf("listen() error\n");
        close(s);
        return -1;
    }
    printf("Listening on 0.0.0.0:%d\n", PORT);

    c = (accept(s, (struct sockaddr*) &srv, &addrlen));
    if (c < 0) {
        printf("accept() error\n");
        close(s);
        return -1;
    }

    printf("Client connected\n");
    memset(buf, 0, 512);
    read(c, buf, 511);
    data = "httpd v1.0\n";
    write(c, data, strlen(data));

    printf("%s\n", buf);

    close(c);
    close(s);

    return 0;
}