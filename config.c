#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void init_config(
    syConfig *conf,
    char *path)
{
    FILE *fp = NULL;
    char line[60];
    char paramName[30];
    char paramValue[30];
    fp = fopen (path, "r");

    if(fp == NULL) {
        perror("fail to open the configuration file");
        exit(EXIT_FAILURE);
    }

    while(fgets (line, 60, fp) != NULL) {
        sscanf(line, "%s %s", paramName, paramValue);

        if(strcmp(paramName, "IP") == 0) {
            strcpy(conf->ip, paramValue);
        }
        if(strcmp(paramName, "PORT") == 0) {
            conf->port = (unsigned short)atoi(paramValue);
        }
        if(strcmp(paramName, "FASTCGI_IP") == 0) {
            strcpy(conf->fastcgiIp, paramValue);
        }
        if(strcmp(paramName, "FASTCGI_PORT") == 0) {
            conf->fastcgiPort = (unsigned short)atoi(paramValue);
        }
        if(strcmp(paramName, "THREAD_NUM") == 0) {
            conf->threadNum = (unsigned short)atoi(paramValue);
        }
        if(strcmp(paramName, "JOB_MAX_NUM") == 0) {
            conf->jobMaxNum = (unsigned short)atoi(paramValue);
        }
    }
    fclose(fp);
}