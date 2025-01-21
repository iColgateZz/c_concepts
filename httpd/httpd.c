/* httpd.c */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define LISTENADDR "127.0.0.1"

/* structures */
typedef struct HttpRequest {
    char method[8];
    char url[128];
} HttpRequest;

/* global */
char* errorDesc;
pid_t originalPid;

/* Return 0 on error, valid socket fd on success. */
int initServer(const int port) {
    int s;
    struct sockaddr_in srv;

    printf("Initializing the server\n");
    printf("Opening a socket\n");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        errorDesc = "socket() error";
        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);

    printf("Binding to port %d\n", port);
    if (bind(s, (struct sockaddr*) &srv, sizeof(srv))) {
        close(s);
        errorDesc = "bind() error";
        return 0;
    }

    if (listen(s, 1024)) {
        close(s);
        errorDesc = "listen() error";
        return 0;
    }
    printf("Listening on %s:%d\n", LISTENADDR, port);

    return s;
}

/* return 0 on error, valid client socket fd on success. */
int acceptClient(const int s) {
    int c;
    socklen_t addrlen;
    struct sockaddr_in cli;

    addrlen = 0;
    memset(&cli, 0, sizeof(cli));

    c = accept(s, (struct sockaddr*) &cli, &addrlen);
    if (c < 0) {
        errorDesc = "accept() error";
        return 0;
    }
    printf("Accepted %d\n", addrlen);

    return c;
}

/* parseHttpRequest helper function. */
HttpRequest* parseHttpRequestError(HttpRequest* req, int val) {
    free(req);
    sprintf(errorDesc, "parseHttpRequest() error %d", val);
    return 0;
}

/* return 0 on error, or an HttpRequest*. */
HttpRequest* parseHttpRequest(const char* str, const int n) {
    HttpRequest* req;
    size_t i, k;
    char* method, *url;

    req = malloc(sizeof(HttpRequest));
    memset(req, 0, sizeof(HttpRequest));

    i = strcspn(str, " ");
    if (i >= n || i > 8 - 1)
        return parseHttpRequestError(req, 1);

    for (int j = 0; j < i; j++)
        req->method[j] = str[j];
    req->method[i] = 0;

    i++;
    if (i >= n)
        return parseHttpRequestError(req, 2);
    
    k = strcspn(str + i, " ");
    if (k >= 128 - 1)
        return parseHttpRequestError(req, 3);

    for (int j = 0; j < k; j++)
        req->url[j] = str[i + j];
    req->url[i + k] = 0;

    return req;
}

char* readClient(const int c, int* len) {
    static char buf[512];

    memset(buf, 0, 512);
    *len = read(c, buf, 512 - 1);
    if (*len == -1) {
        errorDesc = "readClient() error";
        return 0;
    }

    return buf;
}

void sendHttpResponse(int c, int code, char* respMsg, char* cType, char* body) {
    char buf[512];
    int n;

    memset(buf, 0, 512);

    sprintf(buf, 
    "HTTP/1.1 %d %s\n"
    "Server: httpd"
    "Content-type: %s\n"
    "\n"
    "%s\n",
    code, respMsg, cType, body);

    if (write(c, buf, strlen(buf)) < 0) {
        fprintf(stderr, "sendHttpResponse() error\n");
    }
}

void handleClient(const int s, const int c) {
    HttpRequest* req;
    char* p;
    int len;
    char* code;

    p = readClient(c, &len);
    if (!p) {
        fprintf(stderr, "%s\n", errorDesc);
        close(c);
        return;
    }

    req = parseHttpRequest(p, len);
    if (!req) {
        fprintf(stderr, "%s\n", errorDesc);
        close(c);
        return;
    }

    if (!strcmp(req->method, "GET") && !strcmp(req->url, "/")) {
        code = "200";
        execl("sendHttpResponse", "sendHttpResponse", );
        sendHttpResponse(c, 200, "OK", "text/html", "<html><h1>Front page</h1></html>");
    } else {
        code = "404";
        sendHttpResponse(c, 404, "Not found", "text/plain", "Page not found");
    }

    free(req);
    close(c);
}

/*
    netstat -an | grep LISTEN
    to check if the server is running.
*/
int main(int argc, char* argv[]) {
    int s, c;
    char* port;
    pid_t ppid;

    originalPid = getpid();
    printf("Original pid: %d\n", originalPid);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    port = argv[1];
    s = initServer(atoi(port));
    if (!s) {
        fprintf(stderr, "%s\n", errorDesc);
        return -1;
    }

    while (1) {
        c = acceptClient(s);
        if (!c) {
            fprintf(stderr, "%s\n", errorDesc);
            continue;
        }

        int forkVal = fork();
        if (forkVal == 0) {
            handleClient(s, c);
            break;
        } else if (forkVal == -1) {
            fprintf(stderr, "Fork() error");
        }

        if (getpid() != originalPid)
            break;
    }

    close(s);
    return 0;
}