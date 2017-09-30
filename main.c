#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include "fastcgi.h"

#define SERVER_PORT 9000

int main(int argc, char **argv)
{
    unsigned short requestId = 1;
    fcgi_container f;
    f.size = 0;
    f.content = (u_char *)malloc(1);
    fcgi_container *f_container = &f;
    u_char *stdinContent = "quantity=100&item=3047936";
    fcgi_name_value_pair fcgiNvpair[] = {
        {"REQUEST_METHOD", "POST"}, {"CONTENT_TYPE", "application/x-www-form-urlencoded"},
        {"SCRIPT_FILENAME", "/home/www/fastcgi/index.php"}, {"CONTENT_LENGTH", "25"},
        {"HTTP_HOST", "fastcgi.com"}, {"SERVER_NAME", "fastcgi.com"},
        {"REDIRECT_STATUS", "200"}, {"SCRIPT_NAME", "/index.php"},
        {"SERVER_ADDR", "127.0.0.1"}, {"SERVER_PORT", "80"},
        {"HTTP_USER_AGENT", "curl/7.47.0"}, {"HTTP_ACCEPT", "*/*"},
        {"SERVER_PROTOCOL", "HTTP/1.1"}, {"REQUEST_SCHEME", "http"},
        {"GATEWAY_INTERFACE", "CGI/1.1"}, {"SERVER_SOFTWARE", "nginx/1.11.5"},
        {"REMOTE_ADDR", "127.0.0.1"}, {"REMOTE_PORT", "23512"},
        {"REQUEST_URI", "/"},{"QUERY_STRING", "hello=world"}
    };
    fc_begin_request(f_container, requestId, 1, 0);
    fc_params(f_container, requestId, fcgiNvpair, 20);
    fc_params_end(f_container, requestId);
    fc_stdin(f_container, requestId, stdinContent);
    fc_stdin(f_container, requestId, "");
    for(int i=0;i<f_container->size;i++) {
        printf("%x ", *(f_container->content + i));
    }
    printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    int clientSocket;
    char recvbuf[2000];  
    int iDataNum;
    struct sockaddr_in serverAddr;
    if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
       perror("socket");
       return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    //指定服务器端的ip，本地测试：127.0.0.1  
    //inet_addr()函数，将点分十进制IP转换成网络字节序IP  
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
       perror("connect");
       return 1;
    }

    printf("connect with destination host...\n");

    send(clientSocket, f_container->content, f_container->size, 0);
    iDataNum = recv(clientSocket, recvbuf, 2000, 0);
    recvbuf[iDataNum] = '\0';
    printf("recv data of my world is:\n");
    char *response = parse_fc_response(recvbuf, iDataNum);
    printf("%s\n", response);
    close(clientSocket);
    return 0;
}
