/* httpd.c */


/* C libraries */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/errno.h>


/* Custom libraries */
#include "logger.c"


/* Definitions */
#define LISTENADDR "192.168.1.239"
#define MAXLINE 65536


/* structures */
typedef struct HttpRequest {
    char method[8];
    char url[128];
} HttpRequest;


/* global */
char* errorDesc;


/* Return 0 on error, valid socket fd on success. */
int initServer(const int port) {
    int s;
    struct sockaddr_in srv;
    struct in_addr lol;

    loggerInfo("Initializing the server\n");
    loggerInfo("Opening a socket\n");
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        errorDesc = "socket() error";
        return 0;
    }

    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(LISTENADDR);

    loggerInfo("Binding to port %d\n", port);
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
    loggerInfo("Listening on %s:%d\n\n-----------------------------\n\n", LISTENADDR, port);

    return s;
}

/* return 0 on error, valid client socket fd on success. */
int acceptClient(const int s) {
    int c;
    struct sockaddr_in cli;
    socklen_t addrlen;
    char addr[32];

    memset(&cli, 0, sizeof(cli));
    memset(addr, 0, 32);
    addrlen = sizeof(cli);

    c = accept(s, (struct sockaddr*) &cli, &addrlen);
    if (c < 0) {
        errorDesc = "accept() error";
        return 0;
    }

    getpeername(c, (struct sockaddr*)&cli, &addrlen);
    inet_ntop(AF_INET, &cli.sin_addr, addr, sizeof(addr));

    loggerAttention("Client connection: %s\n", addr);

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

    if (!n) return parseHttpRequestError(req, 0);

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
    char* buf;
    int ret;
    fd_set rfds;
    struct timeval tv;

    buf = malloc(MAXLINE);
    memset(buf, 0, MAXLINE);
    *len = 0;

    /* read() blocks for 1 second at most. */
    memset(&tv, 0, sizeof(tv));
    tv.tv_usec = 0;
    tv.tv_sec = 1;

    FD_ZERO(&rfds);
    FD_SET(c, &rfds);

    ret = select(c + 1, &rfds, 0, 0, &tv);

    if (ret && FD_ISSET(c, &rfds))
        *len = read(c, buf, MAXLINE - 1);

    loggerWarning("Len: %d\n", *len);
    if (*len <= 0) {
        errorDesc = "readClient() error";
        return 0;
    }

    return buf;
}

void sendHttpResponse(int c, int code, char* respMsg, char* cType, char* body) {
    char buf[MAXLINE];
    int n;

    memset(buf, 0, MAXLINE);

    sprintf(buf, 
    "HTTP/1.1 %d %s\n"
    "Server: httpd\n"
    "Content-type: %s\n"
    "\n"
    "%s\n",
    code, respMsg, cType, body);

    if (write(c, buf, strlen(buf)) < 0) {
        loggerError(stderr, "sendHttpResponse() error: %d\n", errno);
    }
}

void handleClient(const int c) {
    HttpRequest* req;
    char* p;
    int len;

    p = readClient(c, &len);
    if (!p) {
        loggerError(stderr, "%s: %d\n", errorDesc, errno);
        close(c);
        return;
    }

    loggerInfo("%s\n", p);

    req = parseHttpRequest(p, len);
    if (!req) {
        loggerError(stderr, "%s: %d\n", errorDesc, errno);
        free(p);
        close(c);
        return;
    }

    if (!strcmp(req->method, "GET") && !strcmp(req->url, "/")) {
        sendHttpResponse(c, 200, "OK", "text/html", "<html><h1>Front page</h1></html>");
    } else {
        sendHttpResponse(c, 404, "Not found", "text/plain", "Page not found");
    }

    free(req);
    free(p);
    close(c);
}

/*
    netstat -an | grep LISTEN
    to check if the server is running.

    To look for memory leaks, run the program,
    in the other terminal window find the pid,
    use "leaks <pid>"
*/
int main(int argc, char* argv[]) {
    int s, c, f;
    char* port;

    if (argc < 2) {
        loggerError(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    port = argv[1];
    s = initServer(atoi(port));
    if (!s) {
        loggerError(stderr, "%s: %d\n", errorDesc, errno);
        return -1;
    }

    while (1) {
        c = acceptClient(s);
        if (!c) {
            loggerWarning("%s: %d\n", errorDesc, errno);
            continue;
        }

        /* If > 2000 calls then fork EAGAIN error (35). */
        f = fork();
        if (f == 0) {
            handleClient(c);
            break;
        } else if (f == -1) {
            loggerWarning("Fork() error: %d\n", errno);
        }
        close(c);
    }

    close(s);
    return 0;
}