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
#include "logger/logger.c"
#include "stringlib/stringlib.c"
#include "htable/htable.c"

/* Definitions */
#define LISTENADDR "192.168.1.239"
#define MAXLINE 65536
#define HEADER_BUF_SIZE 256

/* Structures */
typedef struct HttpRequest {
    char method[8];
    char url[128];
} HttpRequest;

/* Global */
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
HttpRequest* parseHttpRequestError(HttpRequest* req, char* errorMsg) {
    free(req);
    sprintf(errorDesc, "parseHttpRequest() error: %s", errorMsg);
    return 0;
}

/* return 0 on error, or an HttpRequest*. */
HttpRequest* parseHttpRequest(char* str, int n) {
    HttpRequest* req;
    char* p;
    int len;
    ht_hash_table* htable;
    char headerKey[HEADER_BUF_SIZE], headerVal[HEADER_BUF_SIZE];

    req = malloc(sizeof(HttpRequest));
    memset(req, 0, sizeof(HttpRequest));
    memset(headerKey, 0, HEADER_BUF_SIZE);
    memset(headerVal, 0, HEADER_BUF_SIZE);
    len = n;
    htable = ht_new();

    /* Parse the starting line. */
    p = copyUntilChar(str, req->method, ' ', 8, &len);
    if (!p) return parseHttpRequestError(req, "EOL after method");

    p = copyUntilChar(p, req->url, ' ', 128, &len);
    if (!p) return parseHttpRequestError(req, "EOL after url");

    /* 
        Skip over the HTTP version.
        The headers are actually optional, 
        but for now I assume they are required.
    */
    p = copyUntilChar(p, NULL, '\n', 0, &len);
    if (!p) return parseHttpRequestError(req, "EOL after HTTP version");

    /* Parse the headers. */
    while (p) {
        /* key */
        p = copyUntilChar(p, headerKey, ':', HEADER_BUF_SIZE, &len);
        if (!p) return parseHttpRequestError(req, "EOL after key");
        p++; len--; /* skip over whitespace */
        if (!p) return parseHttpRequestError(req, "EOL after key");

        /* val */
        p = copyUntilChar(p, headerVal, '\r', HEADER_BUF_SIZE, &len);
        p++; len--; /* skip over \n */
        if (!p) return parseHttpRequestError(req, "EOL after val");

        ht_insert(htable, headerKey, headerVal);
        if (*p == '\r') break;
    }
    ht_print_table(htable);

    /* Parse the body. */

    ht_del_hash_table(htable);

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