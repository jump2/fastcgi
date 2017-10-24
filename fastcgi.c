#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fastcgi.h"

void fc_alloc(fcgi_container *fc, size_t addSize, int c)
{
    size_t old_size = fc->size;
    size_t content_size = old_size + addSize;
    fc->content = (u_char *)realloc(fc->content, content_size);
    memset(fc->content + old_size, c, addSize);
    fc->size = content_size;
}

void fc_container_cat(fcgi_container *fc, fcgi_container *fc2)
{
    size_t oldSize = fc->size;
    fc_alloc(fc, fc2->size, 0);
    u_char *p = fc->content + oldSize;
    for(int i = 0; i < fc2->size; i++) {
        *(p++) = *(fc2->content++);
    }
    free(fc2);
}

void build_header(fcgi_container *fc, u_char type, u_short requestId, u_short contentLength)
{
    size_t oldSize = fc->size;
    fc_alloc(fc, CONTENT_HEADER_LENGTH, 0);
    u_char *p = fc->content + oldSize;
    *(p++) = (u_char)FCGI_VERSION;
    *(p++) = (u_char)type;
    *(p++) = (u_char)(requestId >> 8);
    *(p++) = (u_char)(requestId & 0xFF);
    *(p++) = (u_char)(contentLength >> 8);
    *p = (u_char)(contentLength & 0xFF);
}

void fc_begin_request(fcgi_container *fc, u_short requestId, u_char role, u_char flags)
{
    build_header(fc, FCGI_BEGIN_REQUEST, requestId, 8);
    size_t oldSize = fc->size;
    fc_alloc(fc, 8, 0);
    u_char *p = fc->content + oldSize;
    *(p++) = (u_char)(role >> 8);
    *(p++) = (u_char)(role & 0xFF);
    *p = (u_char)(flags);
}

void fc_params(fcgi_container *fc, u_short requestId, fcgi_name_value_pair params[], size_t len)
{
    fcgi_container *fc2 = process_params(params, len);
    build_header(fc, FCGI_PARAMS, requestId, fc2->size);
    fc_container_cat(fc, fc2);
}

void fc_params_end(fcgi_container *fc, u_short requestId)
{
    build_header(fc, FCGI_PARAMS, requestId, 0);
}

fcgi_container *process_params(fcgi_name_value_pair params[], size_t len)
{
    u_char *requestParams;
    size_t paramsLength;
    int index = 0;
    for(int i = 0; i < len; i++) {
        int nameLength = strlen(params[i].name);
        int valueLength = strlen(params[i].value);
        if(nameLength < 128) {
            paramsLength += (1 + nameLength);
        } else {
            paramsLength += (4 + nameLength);
        }

        if(valueLength < 128) {
            paramsLength += (1 + valueLength);
        } else {
            paramsLength += (4 + valueLength);
        }

        if(i == 0) {
            requestParams = (u_char *)malloc(paramsLength);
        } else {
            requestParams = (u_char *)realloc(requestParams, paramsLength);
        }
        if(nameLength < 128) {
            requestParams[index++] = nameLength; 
        } else {
            requestParams[index++] = nameLength >> 24;
            requestParams[index++] = nameLength >> 16 & 0xFF;
            requestParams[index++] = nameLength >> 8 & 0xFF;
            requestParams[index++] = nameLength & 0xFF;
        }
        if(valueLength < 128) {
            requestParams[index++] = valueLength;
        } else {
            requestParams[index++] = valueLength >> 24;
            requestParams[index++] = valueLength >> 16 & 0xFF;
            requestParams[index++] = valueLength >> 8 & 0xFF;
            requestParams[index++] = valueLength & 0xFF;
        }
        strcpy(requestParams+index, params[i].name); 
        index += nameLength;
        strcpy(requestParams+index, params[i].value);
        index += valueLength;
    }
    fcgi_container *fc = (fcgi_container *)malloc(sizeof(fcgi_container));
    fc->size = index;
    fc->content = requestParams;
    return fc;
}

void fc_stdin(fcgi_container *fc, u_short requestId, u_char *content)
{
    size_t contentLength = strlen(content);
    build_header(fc, FCGI_STDIN, requestId, contentLength);
    fcgi_container *fc2;
    fc2 = (fcgi_container *)malloc(sizeof(fcgi_container));
    fc2->size = contentLength;
    fc2->content = content;
    fc_container_cat(fc, fc2);
}

char *parse_fc_response(char response[], size_t length)
{
    char *html, *p;
    if(response[1] == FCGI_STDOUT) {
        size_t contentLength = (u_char)response[4]<<8 | (u_char)response[5];
        html = malloc(contentLength + 1);
        p = html;
        for(int i = 8; i < 8 + contentLength; i++) {
            *(p++) = response[i];
        }
        p = '\0';
        return html;
    }
    return '\0';
}
