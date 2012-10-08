/* geekhttpd main file 
 *
 * date: 2010/4/15
 * author: Chengdong.Lee
 * mail: brytonlee01@gmail.com
 *
 */

#include	"./config/config.h"
#include	"./daemon/daemon.h"
#include	"./net/socket.h"
#include	"./thread/task.h"

/*---------------------- main ----------------------------------*/
int main(int argc, char ** argv)
{
	int	listenfd; // 监听文件描述符
	int	res;	  // 保存函数的返回值

	/* 先载入默认配置，如果用户有更改就使用更改的配置*/
	res = geekhttpd_default_cfg();
	if (-1 == res) 
		return -1;

	if (!(access(GEEKHTTPD_DEFAULT_CONFIG_PATH, R_OK))) {
		res = geekhttpd_read_config(GEEKHTTPD_DEFAULT_CONFIG_PATH);
		if (-1 == res) 
			return -1;
	}
	geekhttpd_cfginfo_print();

	/* daemon 模块*/
	res = geekhttpd_daemon();
	if ( -1 == res )
		goto cfg_cleanup;	

	/* 创建socket 并在指定的端口上进行监听*/
	res = geekhttpd_bind();
	if ( -1 == res )
		goto log_close;

	res = geekhttpd_epoll_create();
	if ( -1 == res )
		goto socket_cleanup;

	/* 创建线程池 */
	res = geekhttpd_create_thread_pool();
	if ( -1 == res ) {
	    syslog(LOG_INFO, "Cannot create thread pool");
		goto socket_cleanup;
	}
	/* 启动监听并处理 */
	syslog(LOG_INFO, "geekhttp started..");
	res = geekhttpd_epoll_wait();
	if ( -1 == res )
		goto socket_cleanup;

	/* SHOULD never come to here! */

socket_cleanup:
	/* TODO */
cfg_cleanup:
	geekhttpd_cfg_free();
log_close:
	closelog();
	return res;
}
