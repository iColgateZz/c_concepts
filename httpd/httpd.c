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
#include <stdbool.h>

/* Custom libraries */
#include "logger/logger.c"
#include "stringlib/stringlib.c"
#include "htable/htable.c"

/* Definitions */
#define LISTENADDR              "0.0.0.0"
#define MAXLINE                 65536 //64kb
#define HEADER_BUF_SIZE         256
#define METHOD_SIZE             8
#define URL_SIZE                1024
#define ALLOWED_CYCLE_NUMBER    5
#define SECONDS_TO_WAIT         10

/* Structures */
typedef struct HttpRequest {
    char method[METHOD_SIZE];
    char url[URL_SIZE];
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
int acceptClient(const int s, char cliIP[INET_ADDRSTRLEN]) {
    int c;
    struct sockaddr_in cli;
    socklen_t addrlen;

    memset(&cli, 0, sizeof(cli));
    memset(cliIP, 0, INET_ADDRSTRLEN);
    addrlen = sizeof(cli);

    c = accept(s, (struct sockaddr*) &cli, &addrlen);
    if (c < 0) {
        errorDesc = "accept() error";
        return 0;
    }

    if (getpeername(c, (struct sockaddr*)&cli, &addrlen) < 0) {
        errorDesc = "getpeername() error";
        return 0;
    }
    
    if (inet_ntop(AF_INET, &cli.sin_addr, cliIP, INET_ADDRSTRLEN) == NULL) {
        errorDesc = "inet_ntop() error";
        return 0;
    }

    return c;
}

/* parseHttpRequest helper function. */
bool parseHttpRequestError(char* errorMsg) {
    sprintf(errorDesc, "parseHttpRequest() error: %s", errorMsg);
    return false;
}

/* return 0 on error, or an HttpRequest*. */
bool parseHttpRequest(char* str, int len, HttpRequest* req, ht_hash_table* headers, char body[MAXLINE]) {
    char* p;
    char headerKey[HEADER_BUF_SIZE]; 
    char headerVal[HEADER_BUF_SIZE];

    memset(req, 0, sizeof(HttpRequest));
    memset(headerKey, 0, HEADER_BUF_SIZE);
    memset(headerVal, 0, HEADER_BUF_SIZE);
    memset(body, 0, MAXLINE);

    /* Parse the starting line. */
    p = copyUntilChar(str, req->method, ' ', METHOD_SIZE, &len);
    if (!p) return parseHttpRequestError("EOL after method");

    p = copyUntilChar(p, req->url, ' ', URL_SIZE, &len);
    if (!p) return parseHttpRequestError("EOL after url");

    /* Skip over the HTTP version. */
    p = copyUntilChar(p, NULL, '\n', 0, &len);
    if (!p) return parseHttpRequestError("EOL after HTTP version");

    /* 
        Parse the headers.
        The headers are actually optional, but
        for now I assume they are always present.
    */
    while (p) {
        /* key */
        p = copyUntilChar(p, headerKey, ':', HEADER_BUF_SIZE, &len);
        if (!p) return parseHttpRequestError("EOL after key");
        p++; len--; /* skip over whitespace */
        if (!p) return parseHttpRequestError("EOL after key");

        /* val */
        p = copyUntilChar(p, headerVal, '\r', HEADER_BUF_SIZE, &len);
        p++; len--; /* skip over \n */
        if (!p) return parseHttpRequestError("EOL after val");

        ht_insert(headers, headerKey, headerVal);
        if (*p == '\r') break;
    }

    /*
        Parse the body if it is present.
        Check the headers for Content-Length.
     */
    if (ht_search(headers, "content-length") != NULL && len > 2) {
        p += 2;
        len = atoi(ht_search(headers, "content-length"));
        strncpy(body, p, len);
    }

    return true;
}

void readClient(const int c, int* len, char request[MAXLINE]) {
    int ret;
    fd_set rfds;
    struct timeval tv;

    *len = 0;

    memset(&tv, 0, sizeof(tv));
    tv.tv_usec = 0;
    tv.tv_sec = SECONDS_TO_WAIT;

    FD_ZERO(&rfds);
    FD_SET(c, &rfds);

    ret = select(c + 1, &rfds, 0, 0, &tv);

    if (ret > 0 && FD_ISSET(c, &rfds))
        *len = read(c, request, MAXLINE - 1);

    loggerWarning("Len: %d\n", *len);
    if (*len <= 0)
        errorDesc = "readClient() error";

    return;
}

void sendHttpResponse(int c, int code, char* respMsg, char* cType, char* body) {
    char buf[MAXLINE];

    memset(buf, 0, MAXLINE);

    sprintf(buf, 
    "HTTP/1.1 %d %s\r\n"
    "Server: httpd\r\n"
    "Content-type: %s\r\n"
    "Content-Length: %zu\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "%s",
    code, respMsg, cType, strlen(body), body);

    if (write(c, buf, strlen(buf)) < 0) {
        loggerError(stderr, "sendHttpResponse() error: %d\n", errno);
    }
}

void handleClient(const int c, char cliIP[INET_ADDRSTRLEN]) {
    HttpRequest req;
    ht_hash_table* headers;
    char body[MAXLINE];
    char request[MAXLINE];
    int len;
    int counter;

    loggerAttention("Client connection: %s\n", cliIP);
    headers = ht_new();
    counter = 0;

    while (counter < ALLOWED_CYCLE_NUMBER) {
        memset(request, 0, MAXLINE);
        readClient(c, &len, request);
        if (len <= 0) {
            loggerError(stderr, "%s: %d\n", errorDesc, errno);
            break;
        }

        loggerInfo("%s\n", request);

        if (!parseHttpRequest(request, len, &req, headers, body)) {
            loggerError(stderr, "%s: %d\n", errorDesc, errno);
            counter++;
            continue;
        }

        if (!strcmp(req.method, "get") && !strcmp(req.url, "/")) {
            sendHttpResponse(c, 200, "OK", "text/html", "<html><h1>Front page</h1></html>");
        } else {
            sendHttpResponse(c, 404, "Not found", "text/plain", "Page not found");
        }
        counter++;
    }

    ht_del_hash_table(headers);
    close(c);
    return;
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
    char cliIP[INET_ADDRSTRLEN];

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
        c = acceptClient(s, cliIP);
        if (!c) {
            loggerWarning("%s: %d\n", errorDesc, errno);
            continue;
        }

        /* If > 2000 calls then fork EAGAIN error (35). */
        f = fork();
        if (f == 0) {
            handleClient(c, cliIP);
            break;
        } else if (f == -1) {
            loggerWarning("Fork() error: %d\n", errno);
        }
        close(c);
    }

    close(s);
    return 0;
}