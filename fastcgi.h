#ifndef __FASTCGI_H__
#define __FASTCGI_H__

#define FCGI_VERSION            1
#define CONTENT_HEADER_LENGTH   8 

typedef unsigned char u_char;
typedef unsigned short u_short;

typedef struct _fcgi_container {
    size_t size;
    u_char *content;
} fcgi_container;

typedef struct _fcgi_name_value_pair {
    unsigned char name[255];
    unsigned char value[255];
} fcgi_name_value_pair;

typedef enum _fcgi_request_type {
    FCGI_BEGIN_REQUEST=  1, /* [in]                              */
    FCGI_ABORT_REQUEST      =  2, /* [in]  (not supported)             */
    FCGI_END_REQUEST        =  3, /* [out]                             */
    FCGI_PARAMS             =  4, /* [in]  environment variables       */
    FCGI_STDIN              =  5, /* [in]  post data                   */
    FCGI_STDOUT             =  6, /* [out] response                    */
    FCGI_STDERR             =  7, /* [out] errors                      */
    FCGI_DATA               =  8, /* [in]  filter data (not supported) */
    FCGI_GET_VALUES         =  9, /* [in]                              */
    FCGI_GET_VALUES_RESULT  = 10  /* [out]                             */
} fcgi_request_type;

/**
 * role
 */
typedef enum _fcgi_role {
    FCGI_RESPONDER  = 1,
    FCGI_AUTHORIZER = 2,
    FCGI_FILTER = 3
} fcgi_role;

/**
 * protocolStatus
 */
typedef enum _fcgi_protocol_status {
    FCGI_REQUEST_COMPLETE   = 0,        /* 请求的正常结束 */
    FCGI_CANT_MPX_CONN      = 1,        /* 拒绝新请求。这发生在Web服务器通过一条线路向应用发送并发的请求时，后者被设计为每条线路每次处理一个请求。 */
    FCGI_OVERLOADED         = 2,        /* 拒绝新请求。这发生在应用用完某些资源时，例如数据库连接。 */
    FCGI_UNKNOWN_ROLE       = 3         /* 拒绝新请求。这发生在Web服务器指定了一个应用不能识别的角色时。*/
} dcgi_protocol_status;

typedef struct _fcgi_header {
    unsigned char version;              /* FastCGI协议版本 */
    unsigned char type;                 /* FastCGI记录类型，也就是记录执行的一般职能 */
    unsigned char requestIdB1;          /* 记录所属的FastCGI请求 */
    unsigned char requestIdB0;
    unsigned char contentLengthB1;      /* 记录的contentData组件的字节数 */
    unsigned char contentLengthB0;
    unsigned char paddingLength;
    unsigned char reserved;
} fcgi_header;

typedef struct _fcgi_begin_request {
    unsigned char roleB1;
    unsigned char roleB0;
    unsigned char flags;
    unsigned char reserved[5];
} fcgi_begin_request;

typedef struct _fcgi_end_request {
    unsigned char appStatusB3;          /* 应用级别的状态码 */
    unsigned char appStatusB2;
    unsigned char appStatusB1;
    unsigned char appStatusB0;
    unsigned char protocolStatus;       /* 协议级别的状态码 参考： dcgi_protocol_status */
    unsigned char reserved[3];
} fcgi_end_request;

void fc_alloc(fcgi_container *fc, size_t addSize, int c);
void fc_container_cat(fcgi_container *fc, fcgi_container *fc2);
void build_header(fcgi_container *fc, u_char type, u_short requestId, u_short contentLength);
void fc_begin_request(fcgi_container *fc, u_short requestId, u_char role, u_char flags);
void fc_params(fcgi_container *fc, u_short requestId, fcgi_name_value_pair params[], size_t len);
void fc_params_end(fcgi_container *fc, u_short requestId);
fcgi_container *process_params(fcgi_name_value_pair params[], size_t len);
void fc_stdin(fcgi_container *fc, u_short requestId, u_char *content);
char *parse_fc_response(char response[], size_t length);
#endif
