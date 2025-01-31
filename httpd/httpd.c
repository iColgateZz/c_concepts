/* httpd.c */

/* 
    Specs used for the server:
    https://datatracker.ietf.org/doc/html/rfc9112
    https://developer.mozilla.org/en-US/docs/Web/HTTP
*/

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
#define LISTENADDR              "192.168.1.239"
#define MAXLINE                 65536 //64kb
#define HEADER_BUF_SIZE         256
#define METHOD_SIZE             8
#define URI_SIZE                8000
#define SECONDS_TO_WAIT         10

/* Definitions for the parser */
#define BAD_REQUEST             400
#define NO_HEADERS              2
#define NOT_IMPLEMENTED         501
#define URI_TOO_LONG            414

/* Structures */
typedef struct RequestLine {
    char method[METHOD_SIZE];
    char uri[URI_SIZE];
} RequestLine;

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

/* return 0 on error, or an HttpRequest*. */
int parseHttpRequest(char* str, int len, RequestLine* reqLine, ht_hash_table* headers, char body[MAXLINE]) {
    char* p;
    char headerKey[HEADER_BUF_SIZE]; 
    char headerVal[HEADER_BUF_SIZE];

    memset(reqLine, 0, sizeof(RequestLine));
    memset(headerKey, 0, HEADER_BUF_SIZE);
    memset(headerVal, 0, HEADER_BUF_SIZE);
    memset(body, 0, MAXLINE);

    /* Skip initial empty lines if there are any */
    while ((*str == '\r' || *str == '\n') && *str) {
        str++;
        len--;
    }

    /*  A recipient that receives whitespace between the start-line and 
        the first header field MUST either reject the message as invalid or ...
        The server SHOULD respond with a 400 (Bad Request) response 
        and close the connection.  */
    if (*str == ' ') {
        errorDesc = "parseHttpRequest() error: whitespace after start-line";
        return BAD_REQUEST;
    }

    /* Parse the request line. */
    p = copyUntilChar(str, reqLine->method, ' ', METHOD_SIZE, &len);
    if (!p) {
        errorDesc = "parseHttpRequest() error: EOL after method";
        return BAD_REQUEST;
    }

    /*  A server that receives a method longer than any that it implements
        SHOULD respond with a 501 (Not Implemented) status code.  */
    if (*(p - 1) != ' ') {
        errorDesc = "parseHttpRequest() error: method too long";
        return NOT_IMPLEMENTED;
    }

    p = copyUntilChar(p, reqLine->uri, ' ', URI_SIZE, &len);
    if (!p) {
        errorDesc = "parseHttpRequest() error: EOL after uri";
        return BAD_REQUEST;
    }

    /*  A server that receives a request-target longer than any URI it
        wishes to parse MUST respond with a 414 (URI Too Long) status code  */
    if (*(p - 1) != ' ') {
        errorDesc = "parseHttpRequest() error: URI too long";
        return URI_TOO_LONG;
    }

    /* Skip over the HTTP version. */
    p = copyUntilChar(p, NULL, '\n', 0, &len);
    if (!p) return BAD_REQUEST;
    if (len == 2 && *p == '\r' && *(p + 1) == '\n') return NO_HEADERS;

    /* Parse the headers. */
    while (p) {
        /* key */
        p = copyUntilChar(p, headerKey, ':', HEADER_BUF_SIZE, &len);
        if (!p) {
            errorDesc = "parseHttpRequest() error: EOL after key";
            return BAD_REQUEST;
        }
        p++; len--; /* skip over whitespace */
        if (!p) {
            errorDesc = "parseHttpRequest() error: EOL after key";
            return BAD_REQUEST;
        }

        /* val */
        p = copyUntilChar(p, headerVal, '\r', HEADER_BUF_SIZE, &len);
        if (!p) {
            errorDesc = "parseHttpRequest() error: EOL after value";
            return BAD_REQUEST;
        }
        p++; len--; /* skip over \n */
        if (!p) {
            errorDesc = "parseHttpRequest() error: EOL after value";
            return BAD_REQUEST;
        }

        ht_insert(headers, headerKey, headerVal);
        if (*p == '\r') break;
    }

    /*  A server MUST respond with a 400 (Bad Request) status code
        to any HTTP/1.1 request message that lacks a Host header field  */
    if (ht_search(headers, "host") == NULL) {
        errorDesc = "parseHttpRequest() error: no Host header field";
        return BAD_REQUEST;
    }

    /*  Parse the body if it is present.
        Check the headers for Content-Length.  */
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
    memset(request, 0, MAXLINE);
    memset(&tv, 0, sizeof(tv));
    tv.tv_usec = 0;
    tv.tv_sec = SECONDS_TO_WAIT;

    FD_ZERO(&rfds);
    FD_SET(c, &rfds);

    ret = select(c + 1, &rfds, 0, 0, &tv);

    if (ret > 0 && FD_ISSET(c, &rfds))
        *len = read(c, request, MAXLINE - 1);

    loggerWarning("Len: %d\n", *len);
    if (*len < 0)
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
    RequestLine reqLine;
    ht_hash_table* headers;
    char body[MAXLINE];
    char request[MAXLINE];
    int len, keepAlive, parseReturn;

    loggerAttention("Client connected: %s\n", cliIP);
    headers = ht_new();
    keepAlive = 1;

    while (keepAlive) {
        readClient(c, &len, request);
        if (len < 0) {
            loggerError(stderr, "%s: %d\n", errorDesc, errno);
            break;
        } else if (len == 0)
            break;

        loggerInfo("%s\n", request);
        parseReturn = parseHttpRequest(request, len, &reqLine, headers, body);
        loggerError(stderr, "%s\n", errorDesc);

        if (ht_search(headers, "connection") != NULL) {

        }

        if (!strcmp(reqLine.method, "get") && !strcmp(reqLine.uri, "/")) {
            sendHttpResponse(c, 200, "OK", "text/html", "<html><h1>Front page</h1></html>");
        } else {
            sendHttpResponse(c, 404, "Not found", "text/plain", "Page not found");
        }
    }

    loggerAttention("Client disconnected: %s\n", cliIP);
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