/* 
 * Author: Chengdong.Lee
 * Date: 2010/5/24 11:00
 */

#include	"./process.h"
#include	"cgi/cgi.h"

/* 
 * 解析出所请求的方法
 *
 */
void get_request_method(const char * buffer, ghd_method *method)
{
    char	*chr;

    if (buffer == NULL){ 
		syslog(LOG_INFO, "[get_request_method]: buffer is NULL");
		*method = UNKNOWN;
		return;
    }

    chr = (char *)buffer;
    if (*chr == 'G') 
		*method = GET;
    else if (*chr == 'P' && *(chr + 1) == 'O')
		*method = POST;
    else if (*chr == 'H')
		*method = HEAD;
    else if (*chr == 'P' && *(chr +1) == 'U')
		*method = PUT;
    else if (*chr == 'D')
		*method = DELETE;
    else if (*chr == 'T')
		*method = TRACE;
    else if (*chr == 'C')
		*method = CONNECT;
    else if (*chr == 'O')
		*method = OPTIONS;
    else 
		*method = UNKNOWN;
}

/* 
 * Parse Header field 
 * Purpos: 通过这个函数获取你所关心的field 的值
 * Returns:
 * 	NULL
 * 	a pointer 
 */
char * Parse_header_field(ghd_head_field headerfield, p_read_data dataS)
{
    char	*value;
    char	*src;
    char	field[25];
    int		counter;	

    if (dataS == NULL || dataS->len == 0)
		return NULL;

    switch (headerfield) {
	case Accept: 
	    strncpy(field, "Accept: ", sizeof("Accept: ")); 
	    break;
	case Accept_Charset:
	    strncpy(field, "Accept-Charset: ", sizeof("Accept-Charset: "));
	    break;
	case Accept_Encoding:
	    strncpy(field, "Accept-Encoding: ", sizeof("Accept-Encoding: "));
	    break;
	case Accept_Language:
	    strncpy(field, "Accept-Language: ", sizeof("Accept-Language: "));
	    break;
	case Authorization:
	    strncpy(field, "Authorization: ", sizeof("Authorization: "));
	    break;
	case Expect:
	    strncpy(field, "Expect: ", sizeof("Expect: "));
	    break;
	case From:
	    strncpy(field, "From: ", sizeof("From: "));
	    break;
	case Host:
	    strncpy(field, "Host: ", sizeof("Host: "));
	    break;
	case If_Match:
	    strncpy(field, "If-Match: ", sizeof("If-Match: "));
	    break;
	case If_Modified_Since:
	    strncpy(field, "If-Modified-Since: ", sizeof("If-Modified-Since: "));
	    break;
	case If_None_Match:
	    strncpy(field, "If-None-Match: ", sizeof("If-None-Match: "));
	    break;
	case If_Range:
	    strncpy(field, "If-Range: ", sizeof("If-Range: "));
	    break;
	case If_Unmodified_Since:
	    strncpy(field, "If-Unmodified-Since: ", sizeof("If-Unmodified-Since: "));
	    break;

	case Max_Forwards:
	    strncpy(field, "Max-Forwards: ", sizeof("Max-Forwards: "));
	    break;
	case Proxy_Authorization:
	    strncpy(field, "Proxy-Authorization: ", sizeof("Proxy-Authorization: "));
	    break;
	case Range:
	    strncpy(field, "Range: ", sizeof("Range: "));
	    break;
	case Referer:
	    strncpy(field, "Referer: ", sizeof("Referer: "));
	    break;
	case TE:
	    strncpy(field, "TE: ", sizeof("TE: "));
	    break;
	case User_Agent:
	    strncpy(field, "User-Agent: ", sizeof("User-Agent: "));
	    break;
	case Connection:
	    strncpy(field, "Connection: ", sizeof("Connection: "));
	    break;
	case Cache_Control:
	    strncpy(field, "Cache-Control: ", sizeof("Cache-Control: "));
	    break;
	case Content_Length:
	    strncpy(field, "Content-Length: ", sizeof("Content-Length: "));
	    break;
	case Content_Range:
	    strncpy(field, "Content-Range: ", sizeof("Content-Range: "));
	    break;
	default:
	    syslog(LOG_INFO, "[Parse_header_field]: Unknow field");
	    return NULL;
    }
	src = (char *)strcasestr(dataS->data, field);
    if (NULL == src) 
		return NULL;

    counter = 0;
    value = src + strlen(field);
    for (; *value != '\r' && *(value + 1) != '\n'; counter++)
		value++;
    if (counter == 0)
		return NULL;
    if ((value = (char *)malloc(counter)) == NULL){
		syslog(LOG_INFO, "[Parse_header_field]: malloc failed");
		return NULL;
    }
    strncpy(value, src + strlen(field), counter);
    *(value + counter) = '\0';
    return value;
}

		


/* 
 * 简单的安全过滤函数
 */
void arg_filter(char * arg)
{
    char	*src, *dest;

    src = dest = arg;

    /*
     * 强迫请求的文件路径以'/'开头
     */

    if (*arg != '/')
		*arg = '/';

    while (*src != ' ') {
		if (strncmp(src, "/../", 4) == 0) 
			src += 3;
		else if (strncmp(src, "//", 2) == 0)
			src++;
		else 
			*dest++ = *src++;
    }
    *dest = '\0';

    if ( *arg == '/'){
		int len = strlen(arg) - 1;
		memmove(arg, arg + 1, len);
		*(arg + len) = '\0';
	}
    if (arg[0] == '\0' || strcmp(arg, "./") == 0 || strcmp(arg, "./..") == 0)
		strcpy(arg, ".");
}

/*
 *file_type
 */
char * file_type( char *path)
{
    char * cp;
    if (( cp = strrchr(path, '.')) != NULL)
		return cp + 1;
    return " -";
}

/* 
 * isadir
 */
int isadir(char *path)
{
    struct stat	info;

    return (stat(path, &info) != -1 && S_ISDIR(info.st_mode));
}
/* 
 * 获取当前时间， GMT
 * 格式如下: Wed, 05 Jan 2007 11:21:25 GMT
 */
void Get_Date(char * outstr, int size)
{
    time_t	t;
    struct tm *tmp;

    if (outstr == NULL || size < 30) 
		return;
    t = time(NULL);
    tmp = (struct tm *) malloc(sizeof(struct tm));
    if ( tmp == NULL)
		return;
    
    localtime_r(&t, tmp);
    if (tmp == NULL)
		return;
    
    if (strftime(outstr, size, "%a, %d %b %Y %H:%M:%S %Z", tmp) == 0) {
		syslog(LOG_INFO, "strftime error: ");
		return;
    }
    if (tmp != NULL)
		free(tmp);
}
/*
 * gh_response_header
 * Returns:
 * 	0 Ok
 * 	-1 表示出错
 */
int gh_response_header(ptr_response_buffer buf, int code, char *content_type)
{
    char	tmp_str_buf[100];
    int		count;
    int		res;
    char	*mesg;
    char	*date_str;
    char	*server_version= "GeekHttpd/0.1";

    if (buf == NULL) {
		syslog(LOG_INFO, "[gh_response_header] buf is NULL");
		return -1;
    }

    switch(code) {
	case 200:
	    mesg = "OK";
	    break;
	case 400:
	    mesg = "Bad Request";
	    break;
	case 401:
	    mesg = "Unauthorized";
	    break;
	case 403:
	    mesg = "Forbidden";
	    break;
	case 404:
	    mesg = "Not Found";
	    break;
	case 405:
	    mesg = "Not Allowed";
	    break;
	case 500:
	    mesg = "Internal Server Error";
	    break;
	case 501:
	    mesg = "Method Not Implemented";
	    break;
	case 503:
	    mesg = "Server Unavailable";
	    break;
	deafult:
	    mesg = NULL;
	    break;
    }

    count = sprintf(tmp_str_buf, "HTTP/1.1 %d %s\r\n", code, mesg);
    if ( (res = insert_resp_buf(buf, tmp_str_buf, count)) <= 0) {
		syslog(LOG_INFO, "[gh_response_header] buf is too small");
		return -1;
    }
    date_str = (char *)malloc(30);
    if (date_str == NULL)
		syslog(LOG_INFO, "malloc failed");
    Get_Date(date_str, 30);
    count = sprintf(tmp_str_buf, "Data: %s\r\n", date_str);
    if (date_str != NULL)
		free(date_str);
    if ( (res = insert_resp_buf(buf, tmp_str_buf, count)) <= 0) {
		syslog(LOG_INFO, "[gh_response_header] buf is too small");
		return -1;
    }
    count = sprintf(tmp_str_buf, "Server: %s\r\n", server_version);
    if ( (res = insert_resp_buf(buf, tmp_str_buf, count)) <= 0) {
		syslog(LOG_INFO, "[gh_response_header] buf is too small");
		return -1;
    }
    count = sprintf(tmp_str_buf, "Content-Type: %s\r\n", content_type);
    if ( (res = insert_resp_buf(buf, tmp_str_buf, count)) <= 0) {
		syslog(LOG_INFO, "[gh_response_header] buf is too small");
		return -1;
    }
    return 0; /* OK */
}
	    

/* 
 * do_get
 */
void do_get(int sockfd, p_read_data dataS)
{
    char	*request_uri; // 请求的路径
    struct stat	info;
	int		has_arg = 0;	// GET parameters

    if (dataS == NULL || dataS->data == NULL || dataS->len == 0) {
		syslog(LOG_INFO, "[do_get]: dataS is NULL");
		return;
    }

	do {
		/* TODO A HTTP protocol parser is required !!! */
		char *term_pos = NULL;
		request_uri = dataS->data + strlen("GET ");
		term_pos = strchr(request_uri, '?');
		if (!term_pos) {
			term_pos = strchr(request_uri, ' ');
			*term_pos = '\0';
		} else {
			has_arg = 1;
		}
	
	}while(0);
    syslog(LOG_INFO, "[do_get]: request URI: %s", request_uri);

	/* simple security filter */
    arg_filter(request_uri);

    if (has_arg)
	    do_cgi(sockfd, dataS);
    else if ((strcmp(request_uri, ".")) == 0){
		if ((stat("index.html", &info)) != -1)
			do_cat(sockfd, "index.html");
		else if ((stat("index.htm", &info)) != -1)
			do_cat(sockfd, "index.htm");
		else
			do_err_resp(sockfd,403);
    } else if ((stat(request_uri, &info)) == -1){
		syslog(LOG_INFO, "[stat]: path %s err: %s", request_uri, strerror(errno));
		do_err_resp(sockfd,404);
    } else if(isadir(request_uri))
		do_ls(sockfd, request_uri);
    else
		do_cat(sockfd, request_uri);
}

/* 
 * do_err_resp
 */
void do_err_resp(int fd, int code)
{
    ptr_response_buffer buf;
    int			in_size;
    char	*ptr;
    int		res;
    char	*content_type = "text/html";

    if ((buf = resp_buf_malloc(1024)) == NULL) {
		syslog(LOG_INFO, "[do_err_resp] resp_buf_malloc");
		return;
    }
    switch(code) {
	case 400:
	    ptr = _400_response_page;
	    break;
	case 401:
	    ptr = _401_response_page;
	    break;
	case 403:
	    ptr = _403_response_page;
	    break;
	case 404:
	    ptr = _404_response_page;
	    break;
	case 405:
	    ptr = _405_response_page;
	    break;
	case 500:
	    ptr = _500_response_page;
	    break;
	case 501:
	    ptr = _501_response_page;
	    break;
	default:
	    syslog(LOG_INFO, "[do_err_resp] unknow code");
	    return;
    }

    res = gh_response_header(buf, code, content_type);
    if (res != 0) {
		syslog(LOG_INFO, "[do_err_resp] gh_response_header");
		return ;
    }

    /* 这里应当不会出现只写一个字节的情况， 
     * 写简单点， 呵呵。。。。
     */
    in_size = 2;
    res = insert_resp_buf(buf, CRLF, in_size);
    if (res != 2)
		syslog(LOG_INFO, "[do_err_resp] insert_resp_buf");
    
    in_size = strlen(ptr);
    while ( (res = insert_resp_buf(buf, ptr, in_size)) != -1) {
		if (res == 0 || res < in_size) {
			while (write(fd, buf->data, buf->size) == -1) {
				if (errno == EINTR)
					continue;
			}
			buf->counter = 0;
		}
		if (res == in_size)
			break;
		in_size -= res;
		ptr += res;
    }

    while (write(fd, buf->data, buf->counter) == -1) {
		if (errno == EINTR)
			continue;
    }

    buf->counter = 0;
    resp_buf_free(buf);
}

/*
 * do_ls
 */
void do_ls(int fd, char *request_path)
{
    DIR		*dirptr;
    struct dirent	*direntp;
    int		code;
    char	*content_type = "text/html";
    ptr_response_buffer buf;
    int			in_size;
    char	*ptr;
    int		res;
   
    if ((buf = resp_buf_malloc(1024)) == NULL) {
		syslog(LOG_INFO, "[do_ls] resp_buf_malloc");
		return;
    }

    code = 200;
    res = gh_response_header(buf, code, content_type);
    if (res != 0) {
		syslog(LOG_INFO, "[do_ls] gh_response_header");
		return;
    }

    /* 同样， 写简单点 */
    res = insert_resp_buf(buf, CRLF, 2);
    if (res != 2)
		syslog(LOG_INFO, "[do_ls] insert_resp_buf");

    ptr = "<html><head><title>Listing of Directory</title></head> \
		   <body><h1>Listing of Directory</h1>";
    in_size = strlen(ptr);
    while ((res = insert_resp_buf(buf, ptr, in_size)) != -1) {
		if (res == 0 || res < in_size) {
			while (write(fd, buf->data, buf->counter) == -1)
			if (errno == EINTR)
				continue;
			buf->counter = 0;
		}
		if (res == in_size)
			break;
		in_size -= res;
		ptr += res;
    }

    if (( dirptr = opendir(request_path)) != NULL) {
		while(direntp = readdir(dirptr)) {
			in_size = strlen(direntp->d_name);
			ptr = direntp->d_name;

			while ((res = insert_resp_buf(buf, ptr, in_size)) != -1) {
				if (res == 0 || res < in_size) {
					while (write(fd, buf->data, buf->counter) == -1)
					if (errno == EINTR)
						continue;
					buf->counter = 0;
				}
				if (res == in_size)
					break;
				in_size -= res;
				ptr += res;
			}

			in_size = 1;
			ptr = "\n";
			res = insert_resp_buf(buf, ptr, in_size);
			if (res == 0) {
				while (write(fd, buf->data, buf->size) == -1)
					if (errno == EINTR)
						continue;

				buf->counter = 0;
				insert_resp_buf(buf, ptr, in_size);
			}
		}

		while (write(fd, buf->data, buf->counter) == -1)
			if (errno == EINTR)
			continue;
		closedir(dirptr);
    }
	ptr = "</body></html>";
	write(fd, ptr, strlen(ptr));
    buf->counter = 0;
    resp_buf_free(buf);
}

/*
 * do_cat
 */
void do_cat(int fd, char * request_path)
{
    char	*filetype;
    char 	*content_type;
    int		readfd;		/* 读取文件的文件描述符 */
    char	read_buf[512];  /* 读缓冲区 */
    int		bytes;
    int		code = 200;

    ptr_response_buffer buf;
    int			in_size;
    char	*ptr;
    int		res;
    
    if ((buf = resp_buf_malloc(1024)) == NULL) {
		syslog(LOG_INFO, "[do_cat] resp_buf_malloc");
		return;
    }

    filetype = file_type(request_path);
    if (filetype == NULL)
		content_type = "text/plain";

    if (strcmp(filetype, "html") == 0 || strcmp(filetype, "htm") == 0)
		content_type = "text/html";
    else if (strcmp(filetype, "gif") == 0)
		content_type = "image/gif";
    else if (strcmp(filetype, "jpg") == 0)
		content_type = "image/jpeg";
    else if (strcmp(filetype, "jpeg") == 0)
		content_type = "image/jpeg";
    else if (strcmp(filetype, "css") == 0)
		content_type = "text/css";
    else
		content_type = "text/plain";
    
    readfd = open(request_path, O_RDONLY);
    if (readfd == -1){
		syslog(LOG_INFO, "[do_cat]: Cannot open %s", request_path);
		return;
    }

    res = gh_response_header(buf, code, content_type);
    if (res != 0) {
		syslog(LOG_INFO, "[do_cat] gh_response_header");
		return ;
    }

    /* 同样， 写简单点 */
    res = insert_resp_buf(buf, CRLF, 2);
    if (res != 2)
		syslog(LOG_INFO, "[do_cat] insert_resp_buf");

    for (;;) {
		if (( bytes = read(readfd, read_buf, 512)) == -1)
			if (errno == EINTR)
			continue;
		if ( bytes == 0)
			break;
		in_size = bytes;
		ptr = read_buf;
		while ((res = insert_resp_buf(buf, ptr, in_size)) != -1) {
			if (res == 0 || res < in_size) {
				while (write(fd, buf->data, buf->size) == -1) 
					if (errno == EINTR)
						continue;
				buf->counter = 0;
			}
			if (res == in_size)
				break;
			in_size -= res;
			ptr += res;
		}
    }
    while (write(fd, buf->data, buf->counter) == -1)
		if (errno == EINTR)
			continue;
    buf->counter = 0;
    resp_buf_free(buf);
}

/*
 * do_post
 */
void do_post(int fd, p_read_data dataS)
{
    /* TODO: 有时间再实现*/
}

/*
 * do_not_implement
 */
void do_not_implement(int fd)
{
    char	*content_type = "text/html";
    ptr_response_buffer	buf;
    int			in_size;
    char		*ptr;
    int			res;

    if ((buf = resp_buf_malloc(1024)) == NULL) {
		syslog(LOG_INFO, "[do_not_implement] resp_buf_malloc");
		return;
    }
    res = gh_response_header(buf, 501, content_type);
    if (res != 0){
		syslog(LOG_INFO, "[do_not_implement] gh_response_header");
		return ;
    }

    res = insert_resp_buf(buf, CRLF, 2);
    if (res !=2 )
		syslog(LOG_INFO, "[do_not_implement] insert_resp_buf");

    in_size = strlen(_501_response_page);
    ptr = _501_response_page;
    while ( (res = insert_resp_buf(buf, ptr, in_size)) != -1) {
		if (res == 0 || res < in_size) {
			while(write(fd, buf->data, buf->size) == -1)
				if (errno == EINTR)
					continue;
			buf->counter = 0;
		}
		if (res == in_size)
			break;
		in_size -= res;
		ptr += res;
	}

	while (write(fd, buf->data, buf->counter) == -1)
		if (errno == EINTR)
			continue;

	buf->counter = 0;
	resp_buf_free(buf);
}

/*
 * do_head
 */
void do_head(int fd, p_read_data dataS)
{
    char	*request_path;
    struct stat	info;
    ptr_response_buffer	buf;
    int			in_size;
    char		*ptr;
    int			res;
    char		*content_type = "text/html";

    if (dataS == NULL || dataS->data == NULL || dataS->len == 0) {
		syslog(LOG_INFO, "[do_head]: dataS is NULL");
		return;
    }

    arg_filter(dataS->data + strlen("HEAD "));
    request_path = dataS->data + strlen("HEAD ");
    syslog(LOG_INFO, "[do_head]: request path %s", request_path);

    if ((stat(request_path, &info)) == -1) 
		do_err_resp(fd,404);
    else {
		if ((buf = resp_buf_malloc(1024)) == NULL) {
			syslog(LOG_INFO, "[do_head] resp_buf_malloc");
			return ;
		}
		res = gh_response_header(buf, 200, content_type);
		if (res != 0) {
			syslog(LOG_INFO, "[do_head] gh_response_header");
			return ;
		}
		in_size = 2;
		res = insert_resp_buf(buf, CRLF, in_size);
		if (res != 2) 
			syslog(LOG_INFO, "[do_head] insert_resp_buf");
		while (write(fd, buf->data, buf->counter) == -1)
			if (errno == EINTR)
			continue;
		buf->counter = 0;
		resp_buf_free(buf);
    }
}

/* 
 * do_unknow_method
 */
void do_unknown_method(int fd)
{
    ptr_response_buffer buf;
    int			in_size;
    char		*ptr;
    int			res;
    char 	*content_type = "text/html";

    if ((buf = resp_buf_malloc(1024)) == NULL) {
		syslog(LOG_INFO, "[do_unknown_method] resp_buf_malloc");
		return ;
    }

    res = gh_response_header(buf, 405, content_type);
    if (res != 0) {
		syslog(LOG_INFO, "[do_unknown_method] gh_response_header");
		return ;
    }

    in_size = 2;
    res = insert_resp_buf(buf, CRLF, in_size);
    if (res != 2)
		syslog(LOG_INFO, "[do_unknown_method] insert_resp_buf");

    in_size = strlen(_405_response_page);
    ptr = _405_response_page;
    while ( (res = insert_resp_buf(buf, ptr, in_size)) != -1) {
		if (res == in_size)
			break;
		if (res == 0 || res < in_size) {
			while(write(fd, buf->data, buf->size) == -1)
				if (errno == EINTR)
					continue;
			buf->counter = 0;
		}
		in_size -= res;
		ptr += res;
    }
    while (write(fd, buf->data, buf->counter) == -1)
	if (errno == EINTR)
	    continue;
    buf->counter = 0;
    resp_buf_free(buf);
}
