/* geekhttpd_daemon
 *
 * 通过g_geekhttpd_deamon_on 开关来决定是否把geekhttpd
 * 作为一个daemon来执行
 *
 */

#include	"daemon.h"
#include	<errno.h>
#include	<limits.h>
#include	<signal.h>


/* open_max:
 * 	获取系统允许打开的最大文件描述符个数
 */

static long open_max(void)
{
#ifdef	OPEN_MAX
long	openmax = OPEN_MAX;
#else
long	openmax = 0;
#endif

#define	OPEN_MAX_GUESS 256
	
	if(openmax == 0) {
		errno = 0;
		if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
			if (errno == 0)
				openmax = OPEN_MAX_GUESS;
			else
				printf("sysconf error for _SC_OPEN_MAX");
		}
	}
	return openmax;
}

int geekhttpd_daemon(void)
{
	int			fd0, fd1, fd2; /* stdin, stdout, stderr */
	long		openmax, i;
	pid_t		pid;
	char		cwd[256];
	struct	sigaction	sa;	/*信号处理 */

	/*由于子进程继承了父进程的file mode creation mask， 
	* 所以子进程要创建文件时也会用这个mask值进行与之后的结
	* 果来创建文件， 所以为了不影响文件的创建，把它清零 */
	umask(0);

	if ((pid = fork()) < 0){
		printf("cann't fork!\n");
		return -1;
	}
	else if( pid != 0) 	/* parent */
		exit(0);	/* 父进程退出*/
	
	/* 把子进程变以一个session leader, process
	 * group leader, 并且没有控制终端 */
	setsid();
	 
	/* 确保子进程没有分配终端*/
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		printf("sigaction error, SIGHUP");
		return(-1);
	}

	/* TODO: 处理SIGTERM */

	/* 把工作目录切换到服务器所在的工作目录， 防止目前工作目前可能被umount而受影响 */
	if((access(g_geekhttpd_doc_path, F_OK|R_OK|X_OK)) == -1) {
	    perror("Cannot access GEEKHTTP_DOC_PATH");
	    return(-1);
	}
	if (chdir(g_geekhttpd_doc_path) < 0) {
		printf("cannot change work directory to GEEKHTTP_DOC_PATH\n");
		return(-1);
	}

	if (getcwd(cwd, 256) == NULL)
	    printf("getcwd error\n");
	printf("[cwd]: %s\n", cwd);

	/* Close all open file descriptors. */
	openmax = open_max();
	for (i = 0; i< openmax; i++)
		close(i);
	
	/* 把标准输入输出和出错文件描述符重定位到/dev/null */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	/* 到目前为止就不能使用printf来输出信息了，
	 * 只有通过syslog
	 */
	openlog("geekhttpd", LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
		syslog(LOG_ERR, "unexpected file descriptors %d %d %d", \
			fd0, fd1, fd2);
		return(-1);
	}
}
