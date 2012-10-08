/*
 * Author: Chengdong.Lee
 * Date: 2010/5/19 17:50
 */
#include	"./task.h"
#include	"../misc/misc.h"

ghd_task_t* task_queue_head;
ghd_task_t* task_queue_tail;

/* 互斥锁与条件变量 */
pthread_mutex_t		mutex;
pthread_cond_t		cond;
/* POSIX 线程属性 */
pthread_attr_t		attr;

static void *response_thread(void *args)
{
    pthread_t		thread_id;
    int				has_method, sockfd;
    unsigned int	n;
    ghd_task_t*		task;
    ghd_method		method;
    ghd_request_fields_t	Connection_field;
    volatile p_read_data	received_data = NULL;


	// 注册线程意外退出回调函数
	// 强制类型转换，make gcc happy.
    pthread_cleanup_push((void (*)(void *))pthread_mutex_unlock,
			(void *)&mutex);

    thread_id = pthread_self();
    syslog(LOG_INFO, "Thread ID: %ld", thread_id);
    /* 处理线程就在这个while 里循环， 
     * 等待信号， 有信号就处理，处理完毕继续等待
     */
    while ( 1 ) {
		/* 在使用前一定要确保mutex已初始化 */
		pthread_mutex_lock(&mutex);

		while(task_queue_head == NULL)
			pthread_cond_wait(&cond, &mutex);

		sockfd = task_queue_head->sockfd;
		task = task_queue_head;
		task_queue_head = task_queue_head->next;
		if (task_queue_head == NULL)
			task_queue_tail = NULL;
		geekhttpd_task_free(task);

		pthread_mutex_unlock(&mutex);

		received_data = alloc_read_data();
		if (received_data == NULL) {
			syslog(LOG_INFO, "Cannot alloc memory for received_data");
			goto closefd;
		}
		received_data->data = malloc(MAXHEADERLEN);
		if (NULL == received_data->data) {
			syslog(LOG_INFO, "Cannot alloc meomory for receive buffer");
			goto closefd;
		}
		received_data->len = 0;

		has_method = 0;
		/* 读取从客户端发来的清求 */
		while (1) {
			/* 重置errno */
			errno = 0;
			if ( received_data->len < MAXHEADERLEN)
				n = read(sockfd, received_data->data + received_data->len,
						MAXHEADERLEN - received_data->len);
			else
				break;

			if (n < 0) {
				if ( errno == EINTR) {
				continue;
				}
				/* 有问题链接的处理 */
				syslog(LOG_INFO, "brower closed connection");
				goto closefd;
			} else if (n == 0) {
				break;
			} else {
				if ( errno == EAGAIN ) 
					break;
				received_data->len += n;
				if (!has_method && received_data->len > 7){
					get_request_method(received_data->data, &method);
					has_method = 1;
				}

#if 0
				/* GET 或 HEAD 其它方法检查数据的合法性 */
				if (received_data->len == MAXHEADERLEN) {
					if ( (received_data->data[MAXHEADERLEN - 3] & 
								received_data->data[MAXHEADERLEN - 2] &
								received_data->data[MAXHEADERLEN - 1] &
								received_data->data[MAXHEADERLEN - 0]) != 
							0x0d0a0d0a)	
					continue;
				}
				
				if (insert_read_data(&received_data, read_buf, n) == -1) {
					syslog(LOG_INFO, "Cannot insert recived data to recived_data");
					goto closefd;
				}
#endif
			}
		} 

		syslog(LOG_INFO, "Thread ID: %ld", thread_id);
		syslog(LOG_INFO, "[Request] Size %d, Content: %s",  received_data->len, \
			(char *)received_data->data);

		/* 
		 * 根据方法响应请求，向浏览器发送消息和数据
		 */
		switch (method) {
			case GET:
				do_get(sockfd, received_data);
				break;
			case POST:
				do_post(sockfd, received_data);
				break;
			case HEAD:
				do_head(sockfd, received_data);
				break;
			case PUT:
			case DELETE:
			case TRACE:
			case CONNECT:
			case OPTIONS:
				do_not_implement(sockfd);
				break;
			case UNKNOWN:
				/* send 405 */
				do_unknown_method(sockfd);
				break;
			default:
				break;
		}
		/* TODO:
		 * Connection: Keep-Alive 
		 * 没有处理
		 */
		
closefd:
		close(sockfd);
		/* free read_data */
		free_read_data(received_data);
		received_data = NULL;
		syslog(LOG_INFO, "===============================================");
    }
    pthread_cleanup_pop(0);
}

/*
 * 分配request fields节点空间
 */
ghd_request_fields_t* geekhttpd_request_fields_alloc()
{
	ghd_request_fields_t*	fields;

	fields = (ghd_request_fields_t *)malloc(sizeof(ghd_request_fields_t));
	if ( NULL == fields ) 
		return NULL;
	memset(fields, 0x0, sizeof(ghd_request_fields_t));
	return fields;
}

void geekhttpd_request_fields_free(ghd_request_fields_t* fields)
{
	if (fields)
		free (fields);
	fields = NULL;	
}

/* 
 * task memalloc 
 * Returns:
 * 	pointer 
 */
ghd_task_t * geekhttpd_task_alloc()
{
    ghd_task_t * newtask;

    newtask = (ghd_task_t *)malloc(sizeof(ghd_task_t));
	if ( NULL == newtask )
		return NULL;

	memset(newtask, 0x0, sizeof(ghd_task_t));
    return newtask;
}

/* 
 * task free
 */
void geekhttpd_task_free(ghd_task_t * taskptr)
{
	if (taskptr) 
		free(taskptr);

	taskptr = NULL;
}

/* 
 * 添加新任务到任务队列
 */
int geekhttpd_task_insert(ghd_task_t * task)
{
	if (NULL == task)
		return -1;

	/* 添加新任务 */
	pthread_mutex_lock(&mutex);
	if (task_queue_head == NULL) {
		task_queue_head = task;
		task_queue_tail = task;
	} else {
		task_queue_tail->next = task;
		task_queue_tail = task;
	}
	/* 解锁 */
	pthread_mutex_unlock(&mutex);

	return 0;
}

/* 
 * 创建线程组
 * Returns: 
 * 	0: ok
 * 	-1: failed
 */
int geekhttpd_create_thread_pool()
{
	/* 动态数组， 在某些编译器上通不过 */
    pthread_t	threads[g_geekhttpd_pthread_num];
    int		i;	// for index
    int		res;

	/* 初始化任务队列 */
    task_queue_head = task_queue_tail = NULL;

    /* 初始化互斥锁和条件变量 */
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    /* 初始化POSIX 线程属性 */
    pthread_attr_init(&attr);
    /* 设定创建线程时为detach state */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for (i = 0; i < g_geekhttpd_pthread_num; i++) { 
		res = pthread_create(&threads[i], &attr, response_thread, NULL);
		if (res != 0) {
			syslog(LOG_INFO, "gh_create_thread_pool: Cannot create threads");
			return -1;
		}
    }

    return 0;
}

/* 
 * 唤醒等待线程开始处理请求
 */
inline void geekhttpd_start_process()
{
	pthread_cond_signal(&cond);
}
