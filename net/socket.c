#include	"socket.h"

int			g_geekhttpd_listenfd; /* �����ļ������� */
int			g_geekhttpd_epfd;
struct epoll_event	g_geekhttpd_ev, g_geekhttpd_events[20];
ghd_socket_t	*g_geekhttpd_socketlist = NULL;

/* ���÷����� */
int setnonblocking(int fd)
{
    int		opts;
    
    opts = fcntl(fd, F_GETFL);
    if ( opts < 0) {
		syslog(LOG_INFO, "fcntl(sock, F_GETFL)");
		return -1;
    }

    opts = opts | O_NONBLOCK;
    if ( fcntl(fd, F_SETFL, opts) < 0) {
		syslog(LOG_INFO, "fcntl(sock, F_SETFL, opts)");
		return -1;
    }
	return 0;
}

/* ����socket�ڵ�Ŀռ� */
ghd_socket_t* geekhttpd_socket_alloc()
{
	ghd_socket_t*	newsocket;

	newsocket = (ghd_socket_t *)malloc(sizeof(ghd_socket_t));
	if ( NULL == newsocket )
		return NULL;
	newsocket->sockfd = -1;
	newsocket->input_buf = newsocket->output_buf = NULL;
	newsocket->next = NULL;
	return newsocket;
}

/* �ͷ�socket�ڵ�Ŀռ� */
void geekhttpd_socket_free(ghd_socket_t *socketnode)
{
	if (socketnode) {
		if (socketnode->input_buf)
			free(socketnode->input_buf);
		if (socketnode->output_buf)
			free(socketnode->output_buf);
		free(socketnode);
		socketnode = NULL;
	}
}

/* ����ȫ��list�� */
void geekhttpd_socket_insert(ghd_socket_t *socketnode)
{
	if (!socketnode)
		return;

	if ( NULL == g_geekhttpd_socketlist) {
		g_geekhttpd_socketlist = socketnode;
		socketnode->next = NULL;
	}

	socketnode->next = g_geekhttpd_socketlist;
	g_geekhttpd_socketlist = socketnode;
}

/* ��socket list��ɾ�� node */
void geekhttpd_socket_remove(ghd_socket_t *socketnode)
{
	ghd_socket_t*	pnode;

	if (!socketnode || !g_geekhttpd_socketlist)
		return;

	/* ��һ�ε��߼�ò���е�����*/
	if (g_geekhttpd_socketlist->sockfd == socketnode->sockfd) 
		g_geekhttpd_socketlist = g_geekhttpd_socketlist->next;

	for (pnode = g_geekhttpd_socketlist; pnode->next; pnode=pnode->next) {
		if (pnode->next->sockfd == socketnode->sockfd)
			pnode->next = pnode->next->next;
	}
}


int geekhttpd_bind(void)
{
	int		listenfd, res;
    struct sockaddr_in	serveraddr;

    /* ���������ļ������� */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
		syslog(LOG_INFO, "socket error");
		return -1;
    } 

    /* ��socket ���óɷ����� */
    res = setnonblocking(listenfd);
	if ( -1 == res ) {
		close(listenfd);
		return -1;
	}

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(g_geekhttpd_port);
    /* �󶨼����˿� */
    bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (errno == EADDRINUSE || errno == EADDRNOTAVAIL) {
		syslog(LOG_INFO, "bind error: please check your permission");
		close(listenfd);
		return -1;
    }

    if (listen(listenfd, LISTENQ) == -1) {
		syslog(LOG_INFO, "listen error");
		close(listenfd);
		return -1;
    }

	g_geekhttpd_listenfd = listenfd;
    return 0;
}

/*
 * geekhttpd_epoll
 */
int geekhttpd_epoll_create(void)
{
    
    /* ����epoll ר���ļ������� */
    g_geekhttpd_epfd = epoll_create(256);
    if (g_geekhttpd_epfd == -1) {
		syslog(LOG_INFO, "epoll_create return -1");
		return -1;
    }

    /* ��������������ô�����¼����� */
    g_geekhttpd_ev.data.fd = g_geekhttpd_listenfd;
    g_geekhttpd_ev.events = EPOLLIN|EPOLLET;
    if (epoll_ctl(g_geekhttpd_epfd, EPOLL_CTL_ADD, g_geekhttpd_listenfd, &g_geekhttpd_ev) == -1) {
		syslog(LOG_INFO, "epoll_ctl error ");
		return -1;
    }
    return 0;
}

/* 
 * geekhttpd_epoll_wait
 * Returns:
 */
int geekhttpd_epoll_wait(void)
{
    int		nfds; // epoll_wait ���ؿɶ�д���ļ�����������
    int		i; // for index
    int		connfd; // �ͻ��˵�����
    int		sockfd;
    socklen_t	clilen;
    struct sockaddr_in clientaddr;
    ghd_socket_t*	newsocket = NULL;
	ghd_task_t*		newtask = NULL;
    char	*clientaddr_str;
    int		res; 

    for (;;) {
		while(1) {
			nfds = epoll_wait(g_geekhttpd_epfd, g_geekhttpd_events, 20, 500);
			if (nfds < 0 && errno == EINTR)
				continue;
			break;
		}
		if (nfds == -1) {
			syslog(LOG_INFO, "epoll wait failed!"); 
			return -1;
		}
		
		for (i = 0; i < nfds; i++) {
			if (g_geekhttpd_events[i].data.fd == g_geekhttpd_listenfd) {
				memset(&clientaddr, 0, sizeof(clientaddr));
				clilen = 0;
				while (1) {
					errno = 0;
					connfd = accept(g_geekhttpd_listenfd, (struct sockaddr *)&clientaddr, &clilen);
					if (connfd < 0 && errno == EINTR) {
						continue;
					}
					break;
				}
				if (connfd < 0) {
					syslog(LOG_INFO, "accept error: %s", strerror(errno));
					continue;
				}
				res = setnonblocking(connfd);
				if ( -1 == res ) {
					while(1) {
						if ( close(connfd) < 0 && errno == EINTR)
							continue;
						break;
					}
					continue;
				}

				clientaddr_str = inet_ntoa(clientaddr.sin_addr);
				syslog(LOG_INFO, "Connect from: %s", clientaddr_str);

#if 0
				/* ���뵽socketlist */
				newsocket = geekhttpd_socket_alloc();
				if ( NULL == newsocket ) {
					/* TODO: have not enough memory, send 500 error! */
					while(close(sockfd)< 0) {
						if (errno == EINTR)
							continue;
					}	
				}
				newsocket->sockfd = connfd;
				geekhttpd_socket_insert(newsocket);

#endif
				/* ��ӵ�������� */
				newtask = geekhttpd_task_alloc();
				if (NULL == newtask) {
					geekhttpd_socket_remove(newsocket);
					geekhttpd_socket_free(newsocket);
					/* TODO: send 500 error! */
					while(close(sockfd) < 0) {
						if (errno == EINTR)
							continue;
					}
				}
				newtask->sockfd = connfd;
#if 0
				newtask->sockinfo = (void *)newsocket;
#endif
				geekhttpd_task_insert(newtask);
				
				/* ע���������ļ������� */
				g_geekhttpd_ev.data.fd = connfd;
				g_geekhttpd_ev.events = EPOLLIN|EPOLLET;
				epoll_ctl(g_geekhttpd_epfd, EPOLL_CTL_ADD, connfd, &g_geekhttpd_ev);

			}else if (g_geekhttpd_events[i].events & EPOLLIN) {
				if (( sockfd = g_geekhttpd_events[i].data.fd) < 0)
					continue;
				geekhttpd_start_process();
			} 
		} /* for (i = 0; i < nfds; i++) */ 
    } /* for (;; ) */
}
