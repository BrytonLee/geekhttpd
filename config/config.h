/* 
 * geekhttpd configure header file
 *
 */

#ifndef	__GEEKHTTPD_CONFIG_H
#define	__GEEKHTTPD_CONFIG_H

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<pthread.h>
#include	<errno.h>


/* geekhttpd 服务器所需要的启动信息*/

extern int	g_geekhttpd_port;			/* 所使用的端口号 */
extern char	*g_geekhttpd_doc_path;		/* html cgi 文件所在的路径 */
extern int	g_geekhttpd_pthread_num;	/* 需要使用的线程数，跟据需要可以调整服务器的性能*/

/* geekhttpd 默认配置 */
#define		GEEKHTTPD_DEFAULT_CONFIG_PATH	"./config/geekhttpd.conf"
#define		GEEKHTTPD_DEFAULT_PORT			8080
#define		GEEKHTTPD_DEFAULT_DOC_PATH		"./doc"
#define		GEEKHTTPD_DEFAULT_PTHREAD_NUM	3

extern int	geekhttpd_default_cfg(void);
extern int	geekhttpd_read_config(const char *path); 
extern void	geekhttpd_cfginfo_print(void);
extern void	geekhttpd_cfg_free(void);

#endif /* __GEEKHTTPD_CONFIG_H */
