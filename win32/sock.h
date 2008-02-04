/* プロトタイプ */
int	InitSocket(char *port,int portno,char *hostname,char *serveraddr);
int	ConnectHost(char *host,char *port,int portno);
int	SocketClose(int soc);
int	RecvSize(int soc,char *buf,int size,int flags);
int	RecvOneLine(int soc,char **ret_buf);
int	DoSend(int soc,char *buf,int size,int flag);
#ifndef	_WIN32
#else
int SocketSetting();

#endif
#define WSOCK_SELECT	WM_USER + 2	/* ソケットイベントを通知するメッセージ */
