/* 使用POSIX线程去处理客户端发来的请求 
 *
 * Author: Chengdong.Lee 
 * Date: 2010/5/19 16:50
 */
#ifndef __GEEKHTTPD_TASK__
#define	__GEEKHTTPD_TASK__
#include	"../config/config.h"
#include	<sys/syslog.h>
#include	<pthread.h>
#include	"../process/process.h"

/* 允许请求头的最大大小 */
#define	MAXHEADERLEN	2048

/* 任务链节点 */
typedef struct _ghd_task_t{
	int			sockfd;
	void		*sockinfo;
	struct _ghd_task_t	*next;
}ghd_task_t;

/* ==================== Global variables =================== */

extern ghd_task_t* task_queue_head;
extern ghd_task_t* task_queue_tail;

/* 互斥锁与条件变量 */
extern pthread_mutex_t		mutex;
extern pthread_cond_t		cond;
/* POSIX 线程属性 */
extern pthread_attr_t		attr;

/* ====================== Functions ======================= */

/* task 结构体分配 
 * Returns:
 * 	pointer	
 */
ghd_task_t* geekhttpd_task_alloc();

/* task 结构体释放 
 * Returns:
 * 	no returns
 */
void geekhttpd_task_free(ghd_task_t *);

/* 
 * 添加新任务到任务队列
 */
int geekhttpd_task_insert(ghd_task_t * task);

/* 创建线程组 
 * Returns
 * 	0: OK
 * 	-1: failed
 */
int geekhttpd_create_thread_pool();

/* 
 * 唤醒等待线程开始处理请求
 */
inline void geekhttpd_start_process();
#endif /* _THREAD_TASK */
