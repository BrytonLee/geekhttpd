/* 
 * geekhttpd read configure from file 
 *
 */

#include	<fcntl.h>
#include	<sys/stat.h>
#include	"config.h"
#include	"../misc/misc.h"

/* geekhttpd 服务器所需要的启动信息*/

int		g_geekhttpd_port;			/* 所使用的端口号 */
char	*g_geekhttpd_doc_path;		/* html cgi 文件所在的路径 */
int		g_geekhttpd_pthread_num;	/* 需要使用的线程数，跟据需要可以调整服务器的性能*/

/* 解析从配置文件中读取到的信息并写入配置项
 * returns:
 * 0 ok
 * -1 fail
 *
 */
static int parse(char *buf)
{
	int		key_len, value_len;
	char 	*p;	
	char	*tmp; /* 用作临时使用的指针 */

	if( NULL == buf && 0 != *buf)
		goto ret_error;	
	if( (p = strchr(buf, '='))){
		*p++= '\0';  	

		/* 去掉字符中所有的空格*/
		if ( !(key_len = stripspace(buf)) || !(value_len = stripspace(p)) ) {
			/* 至少有一个字符串的长度为空 */
			return 0;
		}	
		
		if ( !(strncmp(buf, "GEEKHTTP_PORT", strlen("GEEKHTTP_PORT")) )) {
			g_geekhttpd_port = str2int(p);
			if (-1 == g_geekhttpd_port) {
				printf("GEEKHTTP_PORT parameter error!\n");
				printf("Use default port: %d\n", GEEKHTTPD_DEFAULT_PORT);
				g_geekhttpd_port == GEEKHTTPD_DEFAULT_PORT;
			}
		}
		else if ( !(strncmp(buf, "GEEKHTTP_DOC_PATH", strlen("GEEKHTTP_DOC_PATH")) )) {
			tmp = (char *)malloc(value_len + 1);
			if ( NULL == tmp) {
				/* 没有分配到内存 */
				printf("Can not alloc enough memory!\n");	
				goto ret_error;	
			}
			
			memset(tmp, 0x0, value_len + 1);
			if( NULL != g_geekhttpd_doc_path)
				free(g_geekhttpd_doc_path);
			g_geekhttpd_doc_path = tmp;
			tmp = NULL;
			strncpy(g_geekhttpd_doc_path, p, value_len);	
		}
		else if ( !(strncmp(buf, "GEEKHTTP_PTHREAD_NUM", strlen("GEEKHTTP_PTHREAD_NUM")) )) {
			g_geekhttpd_pthread_num = str2int(p);
			if ( -1 == g_geekhttpd_pthread_num) {
				printf("GEEKHTTP_PTHREAD_NUM parameter error!\n");
				printf("Use default: %d\n", GEEKHTTPD_DEFAULT_PTHREAD_NUM);
				g_geekhttpd_pthread_num = GEEKHTTPD_DEFAULT_PTHREAD_NUM;
			}
		} else
			goto ret_error;	
	}
	return 0;

ret_error:
	return -1;
}

int geekhttpd_default_cfg(void)
{	
	int		len;

	len = sizeof(GEEKHTTPD_DEFAULT_CONFIG_PATH);
	g_geekhttpd_doc_path = (char *)malloc(len);
	if (NULL == g_geekhttpd_doc_path) {
		printf("memory alloc failed!\n");
		return -1;
	}
	strncpy(g_geekhttpd_doc_path, GEEKHTTPD_DEFAULT_CONFIG_PATH, len);
	g_geekhttpd_port = GEEKHTTPD_DEFAULT_PORT; 
	g_geekhttpd_pthread_num = GEEKHTTPD_DEFAULT_PTHREAD_NUM;
	
	return 0;
}

/* 读取配置文件*/
int	geekhttpd_read_config(const char *path)
{
	FILE	*fp;
	int	n, reval;
	char 	buffer[1024];

	reval = -1;

	if( path == NULL) {
		printf("path is null");
		return reval; /* -1 */
	}

	while( (fp = fopen(path, "r")) == NULL && errno == EINTR) ;
	if (fp == NULL ) {
		printf(" open file failed! \n" );
		return reval; /* -1 */
	}

	while(fgets(buffer, sizeof(buffer), fp)){
		if ( buffer[0] == '#' || !(strchr(buffer, '=')) ){ 
			/* 跳过buffer以'#' 开头或不包含'='号的行 */
			continue;
		}
		if (parse(buffer)){
			printf("configure file parse error\n");
			return reval; /* -1 */
		}
	}

	fclose(fp);

	return (reval = 0);
}

/* 打印 geek http config information */
void geekhttpd_cfginfo_print(void)
{
	printf("g_geekhttpd_port: %d\n", g_geekhttpd_port);
	printf("g_geekhttpd_doc_path: %s\n", g_geekhttpd_doc_path);
	printf("g_geekhttpd_pthread_num: %d\n", g_geekhttpd_pthread_num);
}

void geekhttpd_cfg_free(void)
{
	if (g_geekhttpd_doc_path) {
		free(g_geekhttpd_doc_path);
		g_geekhttpd_doc_path = NULL;
	}
}
