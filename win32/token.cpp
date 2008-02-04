#include "stdafx.h"

#include	"inc.h"
#include	"token.h"


#define TOKEN_ALLOC_SIZE	(128)

//#ifdef	SJIS
#define issjiskanji(c)	((0x81 <= (unsigned char)(c&0xff) && (unsigned char)(c&0xff) <= 0x9f)	\
	|| (0xe0 <= (unsigned char)(c&0xff) && (unsigned char)(c&0xff) <= 0xfc))
//#else
//#define	issjiskanji(c)	0
//#endif

/* �v���g�^�C�v */
static int AddToken(TOKEN *token,char *buf,int len);


/* �g�[�N���̐؂�o�� */
int GetToken(char *buf,int len,TOKEN *token,char *token_separate)
{
int	token_start,token_len;
int	i;

	token->token=NULL;
	token->size=0;
	token->no=0;

	token_start=0;
	for(i=0;i<len;i++){
		/* SJIS����1�o�C�g�� */
		if(issjiskanji(buf[i])){
			i++;
		}
		else if(strchr(token_separate,buf[i])!=NULL){	/* �������� */
			token_len=i-token_start;
			if(token_len>0){
				AddToken(token,&buf[token_start],token_len);
			}
			token_start=i+1;
		}
	}
	token_len=i-token_start;
	if(token_len>0){
		AddToken(token,&buf[token_start],token_len);
	}

	return(0);
}

/* �g�[�N���f�[�^�ǉ� */
static int AddToken(TOKEN *token,char *buf,int len)
{
int	pack_flag;

	pack_flag=0;
	if(buf[0]=='"'&&buf[len-1]=='"'){
		pack_flag=1;
	}
	else if(buf[0]=='\''&&buf[len-1]=='\''){
		pack_flag=1;
	}

	if(token->size==0){
		token->size=TOKEN_ALLOC_SIZE;
		token->token=(char **)malloc(token->size*sizeof(char *));
		token->token[token->no]=(char*)malloc(len+1);
		if(pack_flag==0){
			memcpy(token->token[token->no],buf,len);
			token->token[token->no][len]='\0';
		}
		else{
			memcpy(token->token[token->no],buf+1,len-2);
			token->token[token->no][len-2]='\0';
		}
		token->no++;
	}
	else{
		if(token->no+1>=token->size){
			token->size=TOKEN_ALLOC_SIZE;
			token->token=(char **)realloc((char *)token->token,token->size*sizeof(char *));
		}
		token->token[token->no]=(char*)malloc(len+1);
		if(pack_flag==0){
			memcpy(token->token[token->no],buf,len);
			token->token[token->no][len]='\0';
		}
		else{
			memcpy(token->token[token->no],buf+1,len-2);
			token->token[token->no][len-2]='\0';
		}
		token->no++;
	}

	return(0);
}

/* �g�[�N���f�[�^��� */
int FreeToken(TOKEN *token)
{
int	i;

	for(i=0;i<token->no;i++){
		free(token->token[i]);
	}
	if(token->size>0){
		free(token->token);
	}
	token->token=NULL;
	token->size=0;
	token->no=0;

	return(0);
}

/* �g�[�N���f�[�^�f�o�b�O�p */
int DebugToken(FILE *fp,TOKEN *token)
{
int	i;
char	buf[512];

	for(i=0;i<token->no;i++){
		sprintf(buf,"[%s]",token->token[i]);
		fprintf(fp,"%s",buf);
	}
	fprintf(fp,"\r\n");

	return(0);
}

/* ���O:�l �`���̐؂�o�� */
int GetNameVal(char *buf,int len,char *name,char *val,char *sep)
{
int	name_flag,i,next;

	strcpy(name,"");
	strcpy(val,"");

	name_flag=0;
	for(i=0;i<len;i++){
		if(issjiskanji(buf[i])){
			i++;
			continue;
		}
		else if(strchr(sep,buf[i])!=NULL){
			name_flag=1;
			strncpy(name,buf,i);
			name[i]='\0';
			next=i+1;
			break;
		}
	}
	if(name_flag==0){
		return(0);
	}

	for(i=next;i<len;i++){
		if(!(isascii(buf[i])&&isspace(buf[i]))){
			next=i;
			break;
		}
	}

	strcpy(val,&buf[next]);

	return(1);
}

/* ��������������� */
int CharSmall(char *buf)
{
char	*ptr;

	for(ptr=buf;*ptr!='\0';ptr++){
		*ptr=tolower(*ptr);
	}

	return(0);
}

/* ������CRLF�폜 */
int CutCrLf(char *str)
{
char	*ptr;

	if((ptr=strchr(str,'\r'))!=NULL){
		*ptr='\0';
	}
	else if((ptr=strchr(str,'\n'))!=NULL){
		*ptr='\0';
	}

	return(0);
}

/* �啶������������������ */
int StrCmp(char *a,char *b)
{
char	*aa,*bb;
int	ret;

	aa=strdup(a);
	CharSmall(aa);
	bb=strdup(b);
	CharSmall(bb);

	ret=strcmp(aa,bb);

	free(aa);
	free(bb);

	return(ret);
}
