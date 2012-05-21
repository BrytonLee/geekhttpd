#ifndef __GEEKHTTPD_DAEMON__
#define	__GEEKHTTPD_DAEMON__
/* geekhttpd_daemon
 *
 * 通过g_geekhttpd_daemon_on决定是否把geekhttpd
 * 作为一个daemon来执行
 *
 */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<syslog.h>
#include	<signal.h>
#include	"../config/config.h"

int geekhttpd_daemon(void); 
#endif
