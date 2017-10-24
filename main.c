#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include <string.h>
#include "fastcgi.h"
#include "config.h"
#include "httpd.h"

// int main(int argc, char **argv)
// {
//     syConfig conf;
//     memset(&conf,0, sizeof(syConfig));
//     init_config(&conf, "wow.conf");
//     unsigned short requestId = 1;
//     fcgi_container f;
//     f.size = 0;
//     f.content = (u_char *)malloc(1);
//     fcgi_container *f_container = &f;
//     u_char *stdinContent = "quantity=100&item=3047936";
//     fcgi_name_value_pair fcgiNvpair[] = {
//         {"REQUEST_METHOD", "POST"}, {"CONTENT_TYPE", "application/x-www-form-urlencoded"},
//         {"SCRIPT_FILENAME", "/home/www/fastcgi/index.php"}, {"CONTENT_LENGTH", "25"},
//         {"HTTP_HOST", "fastcgi.com"}, {"SERVER_NAME", "fastcgi.com"},
//         {"REDIRECT_STATUS", "200"}, {"SCRIPT_NAME", "/index.php"},
//         {"SERVER_ADDR", "127.0.0.1"}, {"SERVER_PORT", "80"},
//         {"HTTP_USER_AGENT", "curl/7.47.0"}, {"HTTP_ACCEPT", "*/*"},
//         {"SERVER_PROTOCOL", "HTTP/1.1"}, {"REQUEST_SCHEME", "http"},
//         {"GATEWAY_INTERFACE", "CGI/1.1"}, {"SERVER_SOFTWARE", "nginx/1.11.5"},
//         {"REMOTE_ADDR", "127.0.0.1"}, {"REMOTE_PORT", "23512"},
//         {"REQUEST_URI", "/"},{"QUERY_STRING", "hello=world"}
//     };
//     fc_begin_request(f_container, requestId, 1, 0);
//     fc_params(f_container, requestId, fcgiNvpair, 20);
//     fc_params_end(f_container, requestId);
//     fc_stdin(f_container, requestId, stdinContent);
//     fc_stdin(f_container, requestId, "");
//     // for(int i=0;i<f_container->size;i++) {
//     //     printf("%x ", *(f_container->content + i));
//     // }
//     printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

//     int clientSocket;
//     char recvbuf[2000];  
//     int iDataNum;
//     clientSocket = socket_client(conf.fastcgiIp, conf.fastcgiPort);

//     printf("connect with destination host...\n");

//     send(clientSocket, f_container->content, f_container->size, 0);
//     iDataNum = recv(clientSocket, recvbuf, 2000, 0);
//     recvbuf[iDataNum] = '\0';
//     printf("recv data of my world is:\n");
//     char *response = parse_fc_response(recvbuf, iDataNum);
//     printf("%s\n", response);
//     close(clientSocket);
//     return 0;
// }

int main(int argc, char **argv)
{
    u_short requestId = 1;
    fcgi_container f;
    f.size = 0;
    f.content = (u_char *)malloc(1);
    fcgi_container *f_container = &f;

    char body[1024];
    int server, client;
    u_short paramLength = 0;
    struct sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);  
    fcgi_name_value_pair fcgiNvpair[20];
    syConfig conf;
    memset(&conf,0, sizeof(syConfig));
    init_config(&conf, "wow.conf");
    server = socket_server(conf.port);
    while(1) {
        client = accept(server, (struct sockaddr *)&clientAddr, (socklen_t *)&addrLen);
        if(client < 0) {  
            perror("accept");  
            continue;  
        }
        accept_request(client, fcgiNvpair, &paramLength, body);
        printf("%d\n", paramLength);
        for(int i=0;i<paramLength;i++) {
            printf("%s = %s\n", fcgiNvpair[i].name, fcgiNvpair[i].value);
        }

        fc_begin_request(f_container, requestId, 1, 0);
        fc_params(f_container, requestId, fcgiNvpair, 20);
        fc_params_end(f_container, requestId);
        fc_stdin(f_container, requestId, body);
        fc_stdin(f_container, requestId, "");
        int clientSocket;

        char recvbuf[2000];  
        int iDataNum;
        clientSocket = socket_client(conf.fastcgiIp, conf.fastcgiPort);

        printf("connect with destination host...\n");

        send(clientSocket, f_container->content, f_container->size, 0);
        iDataNum = recv(clientSocket, recvbuf, 2000, 0);
        close(clientSocket);
        recvbuf[iDataNum] = '\0';
        printf("recv data of my world is:\n");
        char *response = parse_fc_response(recvbuf, iDataNum);
        printf("%s\n", response);

        char buf[1024];
        sprintf(buf, "HTTP/1.0 200 OK\r\n");
        send(client, buf, strlen(buf), 0);
        send(client, response, strlen(response), 0);
        close(client);

        free(response);
        f_container->size = 0;
        free(f_container->content);
        f_container->content = (u_char *)malloc(1);
    }
    close(server);
    return 0;
}