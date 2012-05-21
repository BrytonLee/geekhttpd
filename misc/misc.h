/* 
 * 这里的函数主要处理的是一些比较杂的任务。
 * 不是很好分类， 所以把它们放到misc 里。
 */
#ifndef __GEEKHTTPD_MISC__
#define __GEEKHTTPD_MISC__

#include	<stdlib.h>
#include	<string.h>
/*
 * 定义一个结构体用来存放从客户端发来的所有数据。
 */
struct read_data {
    void * data;
    unsigned long len;
    unsigned short flag; // used for insert data
};

typedef struct read_data *p_read_data;

/* 
 * alloc_read_data
 * Returns:
 * 	a pointer 
 */
p_read_data
alloc_read_data();

/*
 * realloc_read_data
 * Purpose: 扩充现在有的结构体大小
 * Returns:
 * 	a pointer
 */
p_read_data
realloc_read_data(p_read_data dataS, unsigned long size);

/* 
 * insert_read_data
 * Purpose: 向结构体里存放数据。
 * Returns:
 * 	0: ok
 * 	-1: failed
 */
int insert_read_data(p_read_data *data_struct, void * data, unsigned long size);
/*
 * free read_data
 */
void free_read_data(p_read_data dataS);

/* 
 * 定义一个结构体用来存放响应输出的数据
 */
struct response_buffer {
    void	* data;
    int		size;
    int		counter;
};

typedef struct response_buffer * ptr_response_buffer;

/* 
 * 分配响应数据缓冲区
 * Returns:
 * 	成功返回一个指针
 * 	失败: NULL
 */
ptr_response_buffer
resp_buf_malloc(int size);

/*
 * 向buffer中写放数据
 * Returns:
 * 	返回写入的数据量
 * 	0 表示buffer 已满
 * 	-1 出错
 */
int
insert_resp_buf(ptr_response_buffer data, void * src, int size);

/*
 * 释放响应数据输出返冲区
 */
void resp_buf_free(ptr_response_buffer buffer);

/* 剥离字符串两端的空格 并返回它的实际长度 */
int stripspace(char *str);
/* 字符串转整型*/
int str2int(char * str);
#endif
