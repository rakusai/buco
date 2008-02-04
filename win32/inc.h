#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<errno.h>
#ifndef _WIN32
#include	<pwd.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/stat.h>
#include	<netinet/in.h>
#include	<netinet/tcp.h>
#include	<netdb.h>
#include	<arpa/inet.h>
#include	<pthread.h>
#include	<sys/wait.h>
#else
#include	<time.h>
#include	<process.h>
#include	<WINSOCK.H>
#define	MAXHOSTNAMELEN	64
#endif
//#include	<varargs.h>
#include <stdarg.h>


/* LogèoóÕ */
int LogPrint(char* fmt , ...);

