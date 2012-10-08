/* 
 * 方法的解析跟处理都在这个文件里定义
 * Author: Chengdong.Lee
 * Date: 2010/05/23 20:56
 */
#ifndef	__GEEKHTTD_PROCESS__
#define	__GEEKHTTPD_PROCESS__

#include	<time.h>
#include	<string.h>
#include	<sys/syslog.h>
#include	<stdio.h>
#include	<errno.h>
#include	<unistd.h>
#include	<dirent.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	"../misc/misc.h"

/* ==================== 数据结构定义 ====================== */
typedef enum{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    TRACE,
    CONNECT, // 保留将来使用。
    OPTIONS,
    UNKNOWN
}ghd_method;

/* Header Field */
typedef enum{
    Accept,
    Accept_Charset,
    Accept_Encoding,
    Accept_Language,
    Accept_Range,
    Age,
    Allow,
    Authorization,
    Cache_Control,
    Connection,
    Content_Encoding,
    Content_Language,
    Content_Length,
    Content_Location,
    Content_MD5,
    Content_Range,   // eg: the first 500 bytes Content-Range: bytes 0-499/1234
    Content_Type,
    Date,
    Etag,
    Expect,
    Expires,
    From,
    Host,
    If_Match,
    If_Modified_Since,
    If_None_Match,
    If_Range,
    If_Unmodified_Since,
    Last_Modified,
    Location,
    Max_Forwards,
    Pargma, //已过时了。
    Proxy_Authenticate,
    Proxy_Authorization,
    Range,
    Referer,
    Retry_After,
    Server,
    TE,
    Trailer,
    Transfer_Encoding,
    Upgrade,
    User_Agent,
    Vary,
    Via,
    Warning,
    WWW_Authenticate,
}ghd_head_field;	/* Header_field  总共47个 */

/* 
 * 没有实现的方法列表
 */
static char *Not_Implemented_Metod[] = {
    "PUT",
    "DELETE",
    "TRACE",
    "CONNECT",
    "OPTIONS",
};

typedef struct _ghd_request_fields_t{
	ghd_method	method;
	char*		uri;
	char*		path_info;
	char*		conten_type;
	int			conten_length;
	/* TODO: 还没有全 */
}ghd_request_fields_t;

/* ===================== 常量与宏 ========================= */
#define	CRLF	"\x0d\x0a"

/*
 * 400
 */
static char	_400_response_page[] =
"<html>" CRLF
"<head><title>400 Bad Request</title></head" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"</body></html>" CRLF
;

/*
 * 401
 */
static char	_401_response_page[] = 
"<html>" CRLF
"<head><title>401 Authorization Required</title><head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>401 Authorization Required</h1></center>" CRLF
"</body></html>" CRLF
;

/*
 * 403
 */
static char	_403_response_page[] =
"<html>" CRLF
"<head><title>403 Forbidden</title></head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>403 Forbidden</h1></center>" CRLF
"</body></html>"
;

/* 
 * 404
 */
static char	_404_response_page[] = 
"<html>" CRLF
"<head><title> 404 Not Found </title></head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>404 Not Found</h1></center>" CRLF
"</body></html>" CRLF
;

/*
 * 405 不被允许的方法
 */
static char	_405_response_page[] =
"<html>" CRLF
"<head><title> 405 Not Allowed</title></head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>405 Not Allowed</h1></center>" CRLF
"</body></html>" CRLF
;

/*
 * 500
 */
static char	_500_response_page[] =
"<html>" CRLF
"<head><title>500 Internal Server Error</title></head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>500 Internal Server Error</h1></center>" CRLF
"</body></html>" CRLF
;

/* 
 * 501 方法还没有实现
 */
static char	_501_response_page[] = 
"<html" CRLF
"<head><title>501 Method Not Implemented</title></head>" CRLF
"<body bgcolor=\"white\">" CRLF
"<center><h1>501 Method Not Implemented</h1></center>" CRLF
"</body></html>" CRLF
;

/* ====================== Functions ======================= */

/*
 * 分配/释放 request fields节点空间
 */
ghd_request_fields_t* geekhttpd_request_fields_alloc();
void geekhttpd_request_fields_free(ghd_request_fields_t *);


/* 解析请求的方法的函数 */
void get_request_method(const char * buffer, ghd_method *method);

/* 
 * Parse Header field 
 * Purpos: 通过这个函数获取你所关心的field 的值
 * Returns:
 * 	NULL
 * 	a pointer 
 */
char * Parse_header_field(ghd_head_field headerfield, p_read_data dataS);

/* 
 * 简单的安全过滤函数
 */
void arg_filter(char * arg);

/*
 * file_type
 */
char *file_type(char *path);

/*
 * isadir
 */
int isadir(char *path);


/* 
 * 获取当前时间， GMT
 * 格式如下: Wed, 05 Jan 2007 11:21:25 GMT
 * Returns:
 * 	check *outstr
 */
void Get_Date(char * outstr, int size);

/* 
 * 设置响应的头信息
 * Note: 用完之后要 free field_ptr 因为这个函数在内部调用malloc
 * 来申请field_ptr的空间
 */
void Set_Response_Header_field(ghd_head_field headerfield, char * field_ptr, char * value);

/*
 * gh_response_header
 */
int gh_response_header(ptr_response_buffer buf, int code, char *content_type);

/*
 * do_get
 */
void do_get(int fd, p_read_data dataS);

/*
 * do_err_resp
 */
void do_err_resp(int fd, int code);

/*
 * do_ls
 */
void do_ls(int fd, char *request_path);

/* 
 * do_cat
 */
void do_cat(int fd, char *request_path);

/*
 * do_post
 */
void do_post(int fd, p_read_data dataS);

/*
 * do_not_implement
 */
void do_not_implement(int fd);

/* 
 * do_head
 */
void do_head(int fd, p_read_data dataS);

/*
 * do_unknow_method
 * send 405
 */
void do_unknown_method(int fd);

#endif /* _METHOD_H */
