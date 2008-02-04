

/* 要求・応答ヘッダ編集指示 */
#define	EH_ADD		1
#define	EH_WRITE	2
#define	EH_DEL		3

/* 要求・応答ライン */
typedef	struct	{
	char	*Method;
	char	*Arg;
	char	*ScriptName;
	char	*Version;
}R_S_LINE;

/* 要求・応答ヘッダ */
typedef struct	{
	int	flag;
	char	*Name;
	char	*Val;
}R_S_HEAD_DATA;
typedef	struct	{
	R_S_HEAD_DATA	*Data;
	int	No;
	int	Alloc;
}R_S_HEAD;

/* 要求・応答コンテント */
typedef	struct	{
	char	*Data;
	int	Size;
	int	Alloc;
}R_S_CONTENT;

/* プロトタイプ */
int	InitLine(R_S_LINE *r_s_line);
int	InitHead(R_S_HEAD *r_s_head);
int	InitContent(R_S_CONTENT *content);
int	InitSet(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	FreeLine(R_S_LINE *r_s_line);
int	FreeHead(R_S_HEAD *r_s_head);
int	FreeContent(R_S_CONTENT *content);
int	FreeSet(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	GetRSLine(char *buf,int len,R_S_LINE *r_s_line);
int	AddHead(R_S_HEAD *r_s_head,char *name,char *val);
int	GetRSHead(char *buf,int len,R_S_HEAD *r_s_head);
int	AddContent(R_S_CONTENT *content,char *data,int len);
int	RecvContent(int soc,R_S_CONTENT *content);
int	RecvChunked(int soc,R_S_CONTENT *content);
int	RecvData(int soc,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	SendData(char s_r,int soc,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	ReadData(char *fname,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	DecodeChunked(R_S_CONTENT *content,char **data,int *data_len);
int	SaveData(FILE *fp,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);
int	HeadSearchName(R_S_HEAD *r_s_head,char *name);
int	EditHead(R_S_HEAD *r_s_head,int flag,char *name,char *val);
int	GetHostFromUrl(char *buf,char *host);
int	GetPortFromUrl(char *buf,char *port);
int	CutHostFromUrl(char *buf);
int	varfgets(char **ret_buf,FILE *fp);
int	GetContentType(char *type,char *exe);
int	DebugData(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content);

