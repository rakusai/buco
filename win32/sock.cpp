#include "stdafx.h"

#include	"inc.h"
#include	"sock.h"


#ifndef	_WIN32
#else
/* WINSOCK初期化 */
int SocketSetting()
{
WORD	wVersionRequested; 
int	winsock_err;
WSADATA	wsaData; 

	wVersionRequested=MAKEWORD(1,1);
	winsock_err=WSAStartup(wVersionRequested,&wsaData); 
	if(winsock_err!=0){
		perror("WSAStartup");
		return(-1);
	}

	return(0);
}
#endif

/* shortバイトオーダ変換 */
static short short_conv(short s)
{
union	{
	short	i;
	struct	{
		unsigned char	a;
		unsigned char	b;
	}s;
}i_s,i_s_ret;

	i_s.i=s;
	i_s_ret.s.a=i_s.s.b;
	i_s_ret.s.b=i_s.s.a;

	return(i_s_ret.i);
}

/* ソケット初期化:サーバ用 */
int InitSocket(char *port,int portno,char *hostname,char *serveraddr)
{
//char	hostname[MAXHOSTNAMELEN];
struct hostent	*myhost;
struct servent	*se;
struct sockaddr_in	me;
//int	opt;
int	p;
int	soc;
struct in_addr	*aptr;

#if defined(_WIN32)
	/* WINSOCK初期化 */
	SocketSetting();
#endif

	if(gethostname(hostname,MAXHOSTNAMELEN)==-1){
		perror("gethostname");
		return(-1);
	}
//	strcpy(servername,hostname);
LogPrint("gethostname=%s\n",hostname);

	if((se=getservbyname(port,"tcp"))==NULL){
/*		perror("getservbyname");*/
	}

	if((myhost=gethostbyname(hostname))==NULL){
		perror("gethostbyname");
		return(-1);
	}
	aptr=(struct in_addr *)*myhost->h_addr_list;
	strcpy(serveraddr,inet_ntoa(*aptr));
LogPrint("gethostbyname=%s\n",serveraddr);
/*
LogPrint("gethostbyname:%d.%d.%d.%d\n",(unsigned char)myhost->h_addr_list[0][0],
								(unsigned char)myhost->h_addr_list[0][1],
								(unsigned char)myhost->h_addr_list[0][2],
								(unsigned char)myhost->h_addr_list[0][3]);
*/

	if((soc=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return(-1);
	}

/*	opt=1;
	if(setsockopt(soc,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeof(int))!=0){
		perror("setsockopt");
		return(-1);
	}
*/
	memset((char *)&me,0,sizeof(me));
	me.sin_family=AF_INET;
	if(se==NULL){
		if((p=atoi(port))==0){
			p=portno;
		}
//#ifdef	X86
		me.sin_port=short_conv(p);
//#else
//		me.sin_port=p;
//#endif
	}
	else{
		me.sin_port = se->s_port;
	}


	if(bind(soc,(struct sockaddr *)&me,sizeof(me))==-1){
		perror("bind");
		return(-1);
	}

	return(soc);
}

/* ソケット接続 */
int ConnectHost(char *host,char *port,int portno)
{
struct hostent	*servhost;
struct servent	*se;
struct sockaddr_in	server;
int	soc,p;
/*
LogPrint("ConnectHost:%s,%s\n",host,port);
*/
	if((servhost=gethostbyname(host))==NULL){
		u_long		addr;
		addr = inet_addr(host);
		servhost = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
		if (servhost == NULL) {
			perror("Error:gethostbyaddr");
			return(-1);
		}
	}
	if((se=getservbyname(port,"tcp"))==NULL){
/*		perror("getservbyname");*/
	}
	if((soc=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return(-1);
	}
	memset((char *)&server,0,sizeof(server));
	server.sin_family=AF_INET;
	if(se==NULL){
		if((p=atoi(port))==0){
			p=portno;
		}
//#ifdef	X86
		server.sin_port=short_conv(p);
//#else
//		server.sin_port=p;
//#endif
	}
	else{
		server.sin_port = se->s_port;
	}
	memcpy((char *)&server.sin_addr,servhost->h_addr,servhost->h_length);
	if(connect(soc,(struct sockaddr *)&server,sizeof(server))==-1){
		perror("connect");
LogPrint("ConnectHost:%s,%s\n",host,port);
		SocketClose(soc);
		return(-1);
	}

	return(soc);
}

/* ソケットクローズ */
int SocketClose(int soc)
{
int	ret;

	shutdown(soc,2);

#ifndef	_WIN32
	ret=close(soc);
#else
	ret=closesocket(soc);
#endif

	return(ret);
}

/* 指定サイズまでソケットから受信 */
int RecvSize(int soc,char *buf,int size,int flags)
{
int	len,lest;
char	*tmpbuf,*ptr;

	lest=size;
	tmpbuf=(char *)malloc(size);
	ptr=tmpbuf;
	while(1){
		len=recv(soc,ptr,lest,0);
		if(len<0){
			perror("recv");
			return(-1);
		}
		lest-=len;
		ptr+=len;
		if(lest<=0){
			break;
		}
	}

	memcpy(buf,tmpbuf,size);
	free(tmpbuf);

	return(size);
}

/* ソケットから１行受信 */
int RecvOneLine(int soc,char **ret_buf)
{
char	*buf;
int	len,size,pos;
char	c;

	*ret_buf=NULL;

	size=0;
	buf=NULL;

	pos=0;
	while(1){
		c='\0';
		len=recv(soc,&c,1,0);
		if(len<=0){
			perror("recv");
			*ret_buf=buf;
			return(pos);
		}
		if(pos+1>=size){
			size+=4096;
			if(buf==NULL){
				buf=(char *)calloc(size,sizeof(char));
			}
			else{
				buf=(char *)realloc(buf,size*sizeof(char));
			}
		}
		buf[pos]=c;pos++;
		if(pos>=1&&buf[pos-1]=='\n'){
			if(pos>=2&&buf[pos-2]=='\r'){
				buf[pos-2]='\0';
				pos-=2;
				break;
			}
			else{
				buf[pos-1]='\0';
				pos--;
				break;
			}
		}
	}

	*ret_buf=buf;
/*
LogPrint("<<<%s>>>\n",buf);
*/
	return(pos);
}

/* ソケット送信 */
int DoSend(int soc,char *buf,int size,int flag)
{
int	ret;
#ifdef	MSGDBG
LogPrint("%s",buf);
#endif
	ret=send(soc,buf,size,flag);

	return(ret);
}

