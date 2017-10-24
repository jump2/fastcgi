#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include "httpd.h"
#include "fastcgi.h"

int socket_server(u_short port)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin))) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, 5)) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int socket_client(char *ip, u_short port)
{
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, &(sin.sin_addr)) <= 0) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void accept_request(int client, fcgi_name_value_pair *fcgiNvpair, u_short *paramNumber, char *body)
{
    size_t bufLen = 1024, contentLen = 0;
    _uri sctUri;
    char buf[bufLen], requestMethod[255], uri[255], serverProtocal[255];
    size_t strLen;
    int index = 0;
    strLen = get_line(client, buf, bufLen);
    sscanf(buf, "%s %s %s", requestMethod, uri, serverProtocal);
    if(strcasecmp(requestMethod, "GET") && strcasecmp(requestMethod, "POST")) {
        unimplemented(client);
        return;
    }
    strcpy(fcgiNvpair[index].name, "REQUEST_METHOD");
    strcpy(fcgiNvpair[index++].value, requestMethod);
    strcpy(fcgiNvpair[index].name, "SERVER_PROTOCOL");
    strcpy(fcgiNvpair[index++].value, serverProtocal);
    strcpy(fcgiNvpair[index].name, "REQUEST_URI");
    strcpy(fcgiNvpair[index++].value, uri);
    //URI
    parse_uri(uri, &sctUri);
    strcpy(fcgiNvpair[index].name, "SCRIPT_FILENAME");
    strcpy(fcgiNvpair[index].value, "/home/www/fastcgi");
    strcat(fcgiNvpair[index++].value, sctUri.path);
    strcpy(fcgiNvpair[index].name, "QUERY_STRING");
    strcpy(fcgiNvpair[index++].value, sctUri.queryString);

    char paramName[128], paramValue[256];
    do {
        strLen = get_line(client, buf, bufLen);
        sscanf(buf, "%[^:]", paramName);
        sscanf(buf, "%*s %[^\n]", paramValue);
        if(strcasecmp(paramName, "HOST") == 0) {
            strcpy(fcgiNvpair[index].name, "HTTP_HOST");
            strcpy(fcgiNvpair[index++].value, paramValue);
            strcpy(fcgiNvpair[index].name, "SERVER_NAME");
            sscanf(paramValue, "%[^:]", fcgiNvpair[index++].value);
        }
        if(strcasecmp(paramName, "CONTENT-TYPE") == 0) {
            strcpy(fcgiNvpair[index].name, "CONTENT_TYPE");
            strcpy(fcgiNvpair[index++].value, paramValue);
        }
        if(strcasecmp(paramName, "CONTENT-LENGTH") == 0) {
            strcpy(fcgiNvpair[index].name, "CONTENT_LENGTH");
            if(strlen(paramValue) == 0) {
                strcpy(fcgiNvpair[index++].value, "0");
            } else {
                strcpy(fcgiNvpair[index++].value, paramValue);
            }
            contentLen = atoi(fcgiNvpair[index-1].value);
        }
        if(strcasecmp(paramName, "USER-AGENT") == 0) {
            strcpy(fcgiNvpair[index].name, "HTTP_USER_AGENT");
            strcpy(fcgiNvpair[index++].value, paramValue);
        }
        if(strcasecmp(paramName, "ACCEPT") == 0) {
            strcpy(fcgiNvpair[index].name, "HTTP_ACCEPT");
            strcpy(fcgiNvpair[index++].value, paramValue);
        }
    } while(strLen);
    strcpy(fcgiNvpair[index].name, "SERVER_ADDR");
    strcpy(fcgiNvpair[index++].value, "127.0.0.1");
    strcpy(fcgiNvpair[index].name, "SERVER_PORT");
    strcpy(fcgiNvpair[index++].value, "8080");
    strcpy(fcgiNvpair[index].name, "REQUEST_SCHEME");
    strcpy(fcgiNvpair[index++].value, "http");
    strcpy(fcgiNvpair[index].name, "SERVER_SOFTWARE");
    strcpy(fcgiNvpair[index++].value, SERVER_SOFTWARE);
    *paramNumber = (u_short)index;

    char c = '\0';
    memset(body, 0, 1024);
    if(contentLen > 0) {
        for(int i=0; i<contentLen; i++) {
            if(recv(client, &c, 1, 0) > 0) {
                body[i] = c;
            }
        }
    }
}

void parse_uri(char *uri, struct uri *sctUri)
{
    printf("%s", uri);
    sscanf(uri, "%[^?]", sctUri->path);
    char *queryStart = strchr(uri, '?');
    if(queryStart != NULL) {
        sscanf(queryStart, "?%[^#]", sctUri->queryString);
    }
    // char *fragmentStart = strchr(uri, "#");
    // if(fragmentStart != NULL) {
    //     sscanf(fragmentStart + 1, "%s", sctUri->fragment);
    // }
}

size_t get_line(int sockfd, char *buf, size_t len)
{
    memset(buf, 0, len);
    char c = '\0';
    for(int i=0; i<len; i++) {
        if(recv(sockfd, &c, 1, 0) > 0) {
            if(c != '\n' && c != '\r') {
                buf[i] = c;   
            } else if(c == '\r' && recv(sockfd, &c, 1, MSG_PEEK) > 0 && c == '\n') {
                recv(sockfd, &c, 1, 0);
                break;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return strlen(buf);
}

void unimplemented(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<html><head><title>501 Not Implemented</title></head>");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<body bgcolor=\"white\">");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<center><h1>501 Not Implemented</h1></center>");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</body></html>\r\n");
    send(client, buf, strlen(buf), 0);
}

void not_found(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}