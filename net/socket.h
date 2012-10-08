/*
 * 网络联接相关模块
 * author: Chengdong.Lee
 * date: 2010/5/19 15:00
 */

#ifndef	_GEEKHTTPD_SOCKET__
#define _GEEKHTTPD_SOCKET__

#include	<sys/socket.h>
#include	<sys/epoll.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<sys/syslog.h>

#include	"../config/config.h"
#include	"../thread/task.h"

#define	OPEN_MAX	100
#define	LISTENQ		20
/* ======================== socket data struct ========================= */

typedef struct _ghd_socket_t {
	int		sockfd;
	char*	input_buf;
	char*	output_buf;
	struct _ghd_socket_t*	next;
}ghd_socket_t;


/* =================== Global variables ==================== */
extern int			g_geekhttpd_listenfd; /* 监听文件描述符 */
extern int			g_geekhttpd_epfd;
extern struct epoll_event	g_geekhttpd_ev, g_geekhttpd_events[20];
extern ghd_socket_t	*g_geekhttpd_socketlist;


/* ================== Functions ========================= */

/* 设置socket 的非阻塞 */
int setnonblocking(int fd);

/* 
 * 分配/释放 socket 节点的空间 */
ghd_socket_t * geekhttpd_socket_alloc();
void geekhttpd_socket_free(ghd_socket_t *);
/* 插入全局list中 */
void geekhttpd_socket_insert(ghd_socket_t *socketnode);
/* 从socket list中删除 node */
void geekhttpd_socket_remove(ghd_socket_t *socketnode);

/* 创建socket 并bind config文件里配置的端口号，
 * 成功返回 0; -1表示失败*/
int  geekhttpd_bind(void);

/*
 * geekhttpd 使用epoll 来监听多文件描述符，
 * epoll 的效率比select 要高得多.
 * Returns:
 * 0: success
 * -1: error */
int geekhttpd_epoll_create(void);

/* 
 * gh_epoll_wait
 * Returns:*/
int geekhttpd_epoll_wait(void);
#endif /* _GEEK_HTTP_SOCKET */
