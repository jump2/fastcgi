#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct
{
    char ip[16];
    unsigned short port;
    char fastcgiIp[16];
    unsigned short fastcgiPort;
    unsigned short threadNum;
    unsigned short jobMaxNum;
} syConfig;

void init_config(syConfig *conf, char *path);
#endif