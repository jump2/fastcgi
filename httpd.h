#ifndef __HTTPD_H__
#define __HTTPD_H__
#include "fastcgi.h"
#define ISspace(x) isspace((int)(x))
#define SERVER_SOFTWARE "wowhttpd/1.0"
#define SERVER_STRING "Server: wowhttpd/1.0\r\n"
typedef struct uri
{
    char path[1024];
    char queryString[255];
} _uri;
int socket_server(u_short port);
int socket_client(char *ip, u_short port);
void accept_request(int client, fcgi_name_value_pair *fcgiNvpair, u_short *paramLength, char *body);
void parse_uri(char *uri, struct uri *sctUri);
size_t get_line(int sockfd, char *buf, size_t len);
void unimplemented(int client);
void not_found(int client);
#endif