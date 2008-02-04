/* トークンデータ */
typedef	struct	{
	char	**token;
	int	size;
	int	no;
}TOKEN;

/* プロトタイプ */
int	GetToken(char *buf,int len,TOKEN *token,char *token_separate);
int	FreeToken(TOKEN *token);
int	DebugToken(FILE *fp,TOKEN *token);
int	GetNameVal(char *buf,int len,char *name,char *val,char *sep);
int	CharSmall(char *buf);
int	CutCrLf(char *str);
int	StrCmp(char *a,char *b);
