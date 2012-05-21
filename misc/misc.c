/* 
 * Author: Chengdong.Lee
 * Date: 2010/5/21 17:00
 */
#include	<sys/syslog.h>
#include	"misc.h"

/* 
 * alloc_read_data
 * Returns:
 * 	a pointer 
 */
p_read_data
alloc_read_data()
{
    p_read_data	dataS;

    dataS = (p_read_data) malloc(sizeof (struct read_data));
    if (dataS == NULL) 
		return NULL;
    dataS->data = NULL;
    dataS->len = 0;
    dataS->flag = 0;

    return dataS;
}

/*
 * realloc_read_data
 * Purpose: ��???????еĽṹ????С
 * Returns:
 * 	a pointer
 */
p_read_data
realloc_read_data(p_read_data dataS, unsigned long size)
{
    p_read_data		RdataS;

    if (size == 0)
		return dataS;
    if (dataS == NULL) {
		RdataS = alloc_read_data(size);
		return RdataS;
    }

    RdataS = alloc_read_data(dataS->len + size);
    if (RdataS == NULL)
	/* memory allocation failed! */
		return NULL;

    /* copy data  */
    memcpy(RdataS->data, dataS->data, dataS->len);
    RdataS->flag = dataS->flag;
    /* free dataS */
    free_read_data(dataS);
    return RdataS;
}


/* 
 * insert_read_data
 * Purpose: ???ṹ???????????ݡ?
 * Returns:
 * 	0: ok
 * 	-1: failed
 */
int insert_read_data(p_read_data *dataS, void * data, unsigned long size)
{
    unsigned long	oldsize;

    if (*dataS == NULL) {
		syslog(LOG_INFO, "[insert_read_data]: dataS is NULL");
		return -1;
    }

    if ((*dataS)->flag == 0) {
		(*dataS)->data = malloc(size);
		if ((*dataS)->data == NULL) {
			syslog(LOG_INFO, "[insert_read_data]: (*dataS)->data is NULL");
			return -1;
		}
		(*dataS)->len = size;
		memcpy((*dataS)->data, data, size);
		(*dataS)->flag = 1;
		return 0;
    }
    oldsize = (*dataS)->len;
    *dataS = realloc_read_data(*dataS, size);
    if (*dataS == NULL)
		return -1;
    memcpy((*dataS)->data + oldsize, data, size);
    return 0;
}
/* 
 * free_read_data
 */
void free_read_data(p_read_data dataS)
{
    if (dataS != NULL) {
	free(dataS->data);
	free(dataS);
    }
}


/* 
 * ??????Ӧ???ݻ?????
 * Returns:
 * 	?ɹ?????һ??ָ??
 * 	ʧ??: NULL
 */
ptr_response_buffer
resp_buf_malloc(int size)
{
    ptr_response_buffer buffer;

    if (size <= 0)
	return NULL;

    buffer = (ptr_response_buffer)malloc(sizeof(struct response_buffer));
    if (buffer == NULL) {
	syslog(LOG_INFO, "[resp_buf_malloc] memory alloc failed");
	return NULL;
    }
    buffer->size = size;
    buffer->counter = 0;
    buffer->data = malloc(size);
    if (buffer->data == NULL) {
	syslog(LOG_INFO, "[resp_buf_malloc] memory alloc failed");
	if (buffer != NULL)
	    free(buffer);
	return NULL;
    }
    return buffer;
}

/*
 * ??buffer??д??????
 * Returns:
 * 	????д????????��
 * 	0 ??ʾbuffer ????
 * 	-1 ????
 */
int
insert_resp_buf(ptr_response_buffer data, void * src, int size)
{
    int		write_size;

    if (src == NULL ||size <= 0 || \
	    data == NULL || data->data == NULL || data->size <= 0)
	return -1;
    if (data->size == data->counter)
	return 0;

    write_size = (data->size - data->counter) < size ? \
		 (data->size - data->counter) : size;
    memcpy(data->data + data->counter, src, write_size);
    data->counter += write_size;

    return write_size;
}


/*
 * ?ͷ???Ӧ???????����???
 */
void resp_buf_free(ptr_response_buffer buffer)
{
    if (buffer != NULL) {
	if (buffer->data != NULL)
	    free(buffer->data);
	free(buffer);
    }
}

/* 
 * 去除字符串头尾的空格
 * return 实际的字符串长度 
 */
int stripspace(char *str)
{
	char 	*p;
	int		len, i;

	if ( NULL == str || NULL == *str)
		goto null_str;
	else { 
		
		len = strlen(str);

		/* 去掉字符串的头空格 */
		for (p = str; isspace(*p) && i < len; p++) 
			*p = '\0';

		/* 去掉字符串的尾空格 */
		for (i = len -1; isspace(*(str+i)) && i >= 0; i--)
			*(str+i) = '\0';
		
		/* 重新计算长度 */
		len = strlen(p);
		if ( 0  == len )
			goto null_str;
		/* 把字符串向前移*/
		memmove(str, p, len);
		*(str+len) = '\0';
		return len;
	}
	
null_str:
	printf("error: str is NULL\n");
	reutrn 0;
}

/* 
 * 实现字符串转换为int
 */
int str2int(char * str)
{
	int	var = 0;
	int	i, len;
	char 	c;

	if (NULL == str || NULL == *str) {
		return -1;
	}

	len = strlen(str);
	for ( i = 0; i < len; i++) {
		c = *(str + i);
		var = var * 10 + (int)(c-48);
	}
	return var;
}
