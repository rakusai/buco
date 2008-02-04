#include "stdafx.h"

#include	"inc.h"
#include	"token.h"
#include	"sock.h"
#include	"http.h"

int LogPrint(char* fmt , ...)
{
va_list args;
char	buf[4096];

	va_start(args,fmt);
	vsprintf(buf,fmt,args);
	va_end(args);

/*
	//ログをファイルに出力します。
	if (hLogFile != INVALID_HANDLE_VALUE){
		DWORD dwWritten;
		WriteFile(hLogFile,buf,strlen(buf),&dwWritten,NULL);

	}
*/
	return 0;
}

/* 要求・応答ラインデータ初期化 */
int InitLine(R_S_LINE *r_s_line)
{
	r_s_line->Method=r_s_line->Arg=r_s_line->Version=NULL;

	return(0);
}

/* 要求・応答ヘッダデータ初期化 */
int InitHead(R_S_HEAD *r_s_head)
{
	r_s_head->Data=NULL;
	r_s_head->No=r_s_head->Alloc=0;

	return(0);
}

/* 要求・応答コンテントデータ初期化 */
int InitContent(R_S_CONTENT *content)
{
	content->Data=NULL;
	content->Size=content->Alloc=0;

	return(0);
}

/* データセット初期化 */
int InitSet(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
	InitLine(r_s_line);
	InitHead(r_s_head);
	InitContent(content);

	return(0);
}

/* 要求・応答ラインデータ解放 */
int FreeLine(R_S_LINE *r_s_line)
{
	if(r_s_line->Method!=NULL){
		free(r_s_line->Method);
	}
	if(r_s_line->Arg!=NULL){
		free(r_s_line->Arg);
	}
	if(r_s_line->Version!=NULL){
		free(r_s_line->Version);
	}

	return(0);
}

/* 要求・応答ヘッダデータ解放 */
int FreeHead(R_S_HEAD *r_s_head)
{
int	i;

	if(r_s_head->No!=0){
		for(i=0;i<r_s_head->No;i++){
			free(r_s_head->Data[i].Name);
			free(r_s_head->Data[i].Val);
		}
		free(r_s_head->Data);
	}

	return(0);
}

/* コンテントデータ解放 */
int FreeContent(R_S_CONTENT *content)
{
	if(content->Size>0&&content->Data!=NULL){
		free(content->Data);
	}

	return(0);
}

/* データセット解放 */
int FreeSet(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
	FreeLine(r_s_line);
	FreeHead(r_s_head);
	FreeContent(content);

	return(0);
}

/* 要求・応答ライン解析 */
int GetRSLine(char *buf,int len,R_S_LINE *r_s_line)
{
TOKEN	token;
int	i;

	InitLine(r_s_line);

	GetToken(buf,len,&token," \t\r\n");

	if(token.no<2){
		FreeToken(&token);
		return(0);
	}

	r_s_line->Method=strdup(token.token[0]);
	r_s_line->Arg=strdup(token.token[1]);
	if(token.no>=3){
		r_s_line->Version=(char *)calloc(strlen(buf),sizeof(char));
		for(i=2;i<token.no;i++){
			if(i!=2){
				strcat(r_s_line->Version," ");
			}
			strcat(r_s_line->Version,token.token[i]);
		}
	}
	else{
		r_s_line->Version=strdup("");
	}

	FreeToken(&token);

	return(1);
}

/* 要求・応答ヘッダ追加 */
int AddHead(R_S_HEAD *r_s_head,char *name,char *val)
{
int	into;

	into=r_s_head->No;
	r_s_head->No++;
	if(r_s_head->Alloc==0){
		r_s_head->Alloc=64;
		r_s_head->Data=(R_S_HEAD_DATA *)calloc(r_s_head->Alloc,sizeof(R_S_HEAD_DATA));
	}
	else if(r_s_head->No>=r_s_head->Alloc){
		r_s_head->Alloc+=64;
		r_s_head->Data=(R_S_HEAD_DATA *)realloc((char *)r_s_head->Data,r_s_head->Alloc*sizeof(R_S_HEAD_DATA));
	}
	r_s_head->Data[into].flag=1;
	r_s_head->Data[into].Name=strdup(name);
	r_s_head->Data[into].Val=strdup(val);

	return(0);
}

/* 要求・応答ヘッダ解析 */
int GetRSHead(char *buf,int len,R_S_HEAD *r_s_head)
{
char	*name,*val;
int	ret;

	name=(char*)calloc(len,sizeof(char));
	val=(char*)calloc(len,sizeof(char));

	if(GetNameVal(buf,len,name,val,":")){
		AddHead(r_s_head,name,val);
		ret=1;
	}
	else{
		ret=0;
	}

	free(name);
	free(val);

	return(ret);
}

/* コンテントデータ追加格納 */
int AddContent(R_S_CONTENT *content,char *data,int len)
{
	if(content->Size+len+1>=content->Alloc){
		if(content->Size+len-content->Alloc>8192){
			content->Alloc+=content->Size+len-content->Alloc;
		}
		else{
			content->Alloc+=8192;
		}
		if(content->Data==NULL){
			content->Data=(char *)malloc(content->Alloc);
		}
		else{
			content->Data=(char *)realloc(content->Data,content->Alloc);
		}
	}
	memcpy(&content->Data[content->Size],data,len);
	content->Data[content->Size+len] = '\0';
	content->Size+=len;

	return(0);
}

/* ソケットからコンテント受信 */
int RecvContent(int soc,R_S_CONTENT *content)
{
int	ret;

	ret=RecvSize(soc,content->Data,content->Size,0);

	return(ret);
}

/* チャンク形式受信 */
int RecvChunked(int soc,R_S_CONTENT *content)
{
int	len;
char	buf2[512],*ptr,*data;
char	*one_data;

	InitContent(content);

	while(1){
		/* サイズ */
		if((len=RecvOneLine(soc,&data))<=0){
			if(data!=NULL){
				free(data);
			}
			LogPrint("A:RecvOneLine:%d\n",len);
			return(-1);
		}
/*
LogPrint("[%s]\n",data);
*/
		AddContent(content,data,len);
		AddContent(content,"\r\n",2);
/*		if(strcmp(data,"0")==0){*/
		if(strtol(data,&ptr,16)==0){
			free(data);
			if((len=RecvOneLine(soc,&data))<0){
				if(data!=NULL){
					free(data);
				}
				LogPrint("B:RecvOneLine:%d\n",len);
				return(-1);
			}
			else{
				AddContent(content,data,len);
/*
LogPrint("Footer[%s]\n",data);
*/
				free(data);
				AddContent(content,"\r\n",2);
			}
			break;
		}
		strcpy(buf2,data);
		free(data);
		if((ptr=strchr(buf2,';'))!=NULL){
			*ptr='\0';
		}
		len=strtol(buf2,&ptr,16);
/*
LogPrint("len=%d\n",len);
*/
		/* データ */
		one_data=(char *)malloc(len);
		RecvSize(soc,one_data,len,0);
		AddContent(content,one_data,len);
		free(one_data);
		if((len=RecvOneLine(soc,&data))<0){
			if(data!=NULL){
				free(data);
			}
			LogPrint("C:RecvOneLine:%d\n",len);
			return(-1);
		}
/*
LogPrint("data-end[%s]\n",data);
*/
		free(data);
		AddContent(content,"\r\n",2);
	}

	return(0);
}


/* 要求・応答データの受信 */
int RecvData(int soc,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
int	len,no,ret;
char	*data;

	/* データセット初期化 */
	InitSet(r_s_line,r_s_head,content);

	/* 要求・応答ライン取得 */
	if((len=RecvOneLine(soc,&data))<=0){
		if(data!=NULL){
			free(data);
		}
		LogPrint("RecvOneLine:%d\n",len);
		return(-1);
	}
	if(!GetRSLine(data,len,r_s_line)){
		LogPrint("GetRSLine:error\n");
		free(data);
		return(-1);
	}
	free(data);
#ifdef	MSGDBG
LogPrint("method=[%s]\n",r_s_line->Method);
LogPrint("arg=[%s]\n",r_s_line->Arg);
LogPrint("version=[%s]\n",r_s_line->Version);
#endif
	/* 要求・応答ヘッダ取得 */
	while(1){
		len=RecvOneLine(soc,&data);
		if(len==0){
			if(data!=NULL){
				free(data);
			}
			break;
		}
		ret=GetRSHead(data,len,r_s_head);
		free(data);
		if(ret==0){
			break;
		}
#ifdef	MSGDBG
LogPrint("[%s]:",r_s_head->Data[r_s_head->No-1].Name);
LogPrint("[%s]\n",r_s_head->Data[r_s_head->No-1].Val);
#endif
	}

	/* コンテント取得 */
	if((no=HeadSearchName(r_s_head,"transfer-encoding"))!=-1){
		/* チャンク形式 */
/*
LogPrint("Transfer-Encoding=%s\n",r_s_head->Data[no].Val);
*/
		if(StrCmp(r_s_head->Data[no].Val,"chunked")==0){
			RecvChunked(soc,content);
		}
	}
	else if((no=HeadSearchName(r_s_head,"content-length"))!=-1){
		/* コンテント長さ指定あり */
/*
LogPrint("Content-Length=%s\n",r_s_head->Data[no].Val);
*/
		content->Size=content->Alloc=atoi(r_s_head->Data[no].Val);
		if(content->Size>0){
			content->Data=(char *)malloc(content->Alloc);
			if(RecvContent(soc,content)==-1){
				FreeContent(content);
				InitContent(content);
				return(-1);
			}
		}
	}
	else{
		/* コンテント指定なし */
	}

	return(0);
}

/* 要求・応答データ送信 */
int SendData(char s_r,int soc,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
int	i,ret;

	ret=DoSend(soc,r_s_line->Method,strlen(r_s_line->Method),0);
	if(ret==-1){
		perror("send");
		return(-1);
	}
	DoSend(soc," ",1,0);
	DoSend(soc,r_s_line->Arg,strlen(r_s_line->Arg),0);
	DoSend(soc," ",1,0);
	DoSend(soc,r_s_line->Version,strlen(r_s_line->Version),0);
	DoSend(soc,"\r\n",2,0);
	for(i=0;i<r_s_head->No;i++){
		if(r_s_head->Data[i].flag==0){
			continue;
		}
		DoSend(soc,r_s_head->Data[i].Name,strlen(r_s_head->Data[i].Name),0);
		DoSend(soc,":",1,0);
		DoSend(soc,r_s_head->Data[i].Val,strlen(r_s_head->Data[i].Val),0);
		DoSend(soc,"\r\n",2,0);
	}
	DoSend(soc,"\r\n",2,0);
	if(content->Size!=0){
		send(soc,content->Data,content->Size,0);
	}

	return(0);
}

/* シナリオから要求データを読み込み */
int ReadData(char *fname,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
char	*data;
FILE	*fp;
long	lnow,lend;
int	ret;

	InitSet(r_s_line,r_s_head,content);

	/* ファイルオープン */
#ifndef	_WIN32
	if((fp=fopen(fname,"r"))==NULL){
#else
	if((fp=fopen(fname,"rb"))==NULL){
#endif
		LogPrint("Cannot read %s\n",fname);
		return(-1);
	}

	/* 要求ライン取得 */
	if(varfgets(&data,fp)==0){
		if(data!=NULL){
			free(data);
		}
		fclose(fp);
		return(-1);
	}
	CutCrLf(data);
	if(!GetRSLine(data,strlen(data),r_s_line)){
		LogPrint("GetRSLine:error\n");
		free(data);
		fclose(fp);
		return(-1);
	}
	free(data);
#ifdef	MSGDBG
LogPrint("method=[%s]\n",r_s_line->Method);
LogPrint("arg=[%s]\n",r_s_line->Arg);
LogPrint("version=[%s]\n",r_s_line->Version);
#endif
	/* 要求ヘッダ取得 */
	while(1){
		if(varfgets(&data,fp)==0){
			if(data!=NULL){
				free(data);
			}
			break;
		}
		CutCrLf(data);
		if(strlen(data)==0){
			free(data);
			break;
		}
		ret=GetRSHead(data,strlen(data),r_s_head);
		free(data);
		if(ret==0){
			break;
		}
#ifdef	MSGDBG
LogPrint("[%s]:",r_s_head->Data[r_s_head->No-1].Name);
LogPrint("[%s]\n",r_s_head->Data[r_s_head->No-1].Val);
#endif
	}

	/* コンテント取得 */
	lnow=ftell(fp);
	fseek(fp,0,SEEK_END);
	lend=ftell(fp);
	fseek(fp,lnow,SEEK_SET);

	content->Size=content->Alloc=lend-lnow;
	if(content->Size!=0){
		content->Data=(char *)malloc(content->Size);
		fread(content->Data,content->Size,1,fp);
	}

	/* ファイルクローズ */
	fclose(fp);

	return(0);
}

/* チャンク形式デコード */
int DecodeChunked(R_S_CONTENT *content,char **data,int *data_len)
{
char	*dptr,*ptr,*dummy;
int	cno,len;

	(*data_len)=0;
	(*data)=(char*)malloc(content->Size);
	dptr=(*data);

	content->Data=(char *)realloc(content->Data,(content->Size+1)*sizeof(char));
	content->Data[content->Size]='\0';

	cno=0;
	while(1){
		if((ptr=strstr(&content->Data[cno],"\r\n"))==NULL){
			break;
		}
		len=strtol(&content->Data[cno],&dummy,16);
/*
LogPrint("len=%d\n",len);
*/
		if(len==0){
			break;
		}
		cno+=ptr-&content->Data[cno]+2;
		memcpy(dptr,&content->Data[cno],len);
		dptr+=len;
		(*data_len)+=len;
		cno+=len;
		cno+=2;
	}

	return(0);
}

/* 要求・応答データ保存 */
int SaveData(FILE *fp,R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
int	i,no;
char	*data;
int	len;

	if(fp==NULL){
		return(-1);
	}

	/* 要求・応答ライン保存 */
	fwrite(r_s_line->Method,strlen(r_s_line->Method),1,fp);
	fwrite(" ",1,1,fp);
	fwrite(r_s_line->Arg,strlen(r_s_line->Arg),1,fp);
	fwrite(" ",1,1,fp);
	fwrite(r_s_line->Version,strlen(r_s_line->Version),1,fp);
	fwrite("\r\n",2,1,fp);
	/* 要求・応答ヘッダ保存 */
	for(i=0;i<r_s_head->No;i++){
		if(r_s_head->Data[i].flag==0){
			continue;
		}
		fwrite(r_s_head->Data[i].Name,strlen(r_s_head->Data[i].Name),1,fp);
		fwrite(":",1,1,fp);
		fwrite(r_s_head->Data[i].Val,strlen(r_s_head->Data[i].Val),1,fp);
		fwrite("\r\n",2,1,fp);
	}
	fwrite("\r\n",2,1,fp);

	/* コンテント保存 */
	if(content->Size!=0){
		if((no=HeadSearchName(r_s_head,"transfer-encoding"))!=-1){
			if(StrCmp(r_s_head->Data[no].Val,"chunked")==0){
				DecodeChunked(content,&data,&len);
				fwrite(data,len,1,fp);
				free(data);
			}
			else{
				fwrite(content->Data,content->Size,1,fp);
			}
		}
		else{
			fwrite(content->Data,content->Size,1,fp);
		}
	}

	return(0);
}

/* ヘッダデータ検索 */
int HeadSearchName(R_S_HEAD *r_s_head,char *name)
{
int	i;
char	*tmpname;
char	buf[80];

	tmpname=strdup(name);
	CharSmall(tmpname);

	for(i=0;i<r_s_head->No;i++){
		strcpy(buf,r_s_head->Data[i].Name);
		CharSmall(buf);
		if(strcmp(buf,tmpname)==0){
			free(tmpname);
			return(i);
		}
	}

	free(tmpname);
	return(-1);
}

/* ヘッダの編集 */
int EditHead(R_S_HEAD *r_s_head,int flag,char *name,char *val)
{
int	no,len;
char	*ptr;

	if(flag==EH_ADD){
		/* 追加 */
		if((no=HeadSearchName(r_s_head,name))!=-1){
			ptr=r_s_head->Data[no].Val;
			len=strlen(ptr)+1+strlen(val)+1;
			r_s_head->Data[no].Val=(char*)calloc(len,sizeof(char));
			sprintf(r_s_head->Data[no].Val,"%s,%s",ptr,val);
			free(ptr);
		}
		else{
			AddHead(r_s_head,name,val);
		}
	}
	else if(flag==EH_WRITE){
		/* 上書き */
		if((no=HeadSearchName(r_s_head,name))!=-1){
			free(r_s_head->Data[no].Val);
			r_s_head->Data[no].Val=strdup(val);
		}
		else{
			AddHead(r_s_head,name,val);
		}
	}
	else if(flag==EH_DEL){
		/* 削除 */
		if((no=HeadSearchName(r_s_head,name))!=-1){
			r_s_head->Data[no].flag=0;
		}
	}

	return(0);
}

/* URLからホストを取得 */
int GetHostFromUrl(char *buf,char *host)
{
char	*s_ptr,*e_ptr;
int	len;

	if((s_ptr=strstr(buf,"://"))==NULL){
		s_ptr=buf;
	}
	else{
		s_ptr=s_ptr+3;
	}

	if((e_ptr=strchr(s_ptr,'/'))==NULL){
		strcpy(host,s_ptr);
	}
	else{
		len=e_ptr-s_ptr;
		strncpy(host,s_ptr,len);
		host[len]='\0';
	}

	if((e_ptr=strchr(host,':'))!=NULL){
		*e_ptr='\0';
	}

	return(0);
}

/* URLからポートを取得 */
int GetPortFromUrl(char *buf,char *port)
{
char	*s_ptr,*e_ptr;
int	len;

	if((s_ptr=strstr(buf,"://"))==NULL){
		s_ptr=buf;
	}
	if((s_ptr=strchr(s_ptr+3,':'))==NULL){
		return(-1);
	}
	if((e_ptr=strchr(s_ptr+1,'/'))==NULL){
		strcpy(port,s_ptr+1);
	}
	else{
		len=e_ptr-s_ptr-1;
		strncpy(port,s_ptr+1,len);
		port[len]='\0';
	}

	return(0);
}
		
/* URLからホストを削除 */
int CutHostFromUrl(char *buf)
{
char	*tmp;
char	*s_ptr,*e_ptr,*q;

	tmp=strdup(buf);

	q = strstr(buf,"?");

	if((s_ptr=strstr(buf,"//"))==NULL || (s_ptr > q && q)){
		s_ptr=buf;
	}
	else{
		s_ptr+=2;
	}
	if((e_ptr=strchr(s_ptr,'/'))==NULL || (e_ptr > q && q)){
		strcpy(tmp,s_ptr);
	}
	else{
		strcpy(tmp,e_ptr);
	}

	strcpy(buf,tmp);
	free(tmp);

	return(0);
}

/* 動的ファイル１行読込み */
int varfgets(char **ret_buf,FILE *fp)
{
char	data[1024],*buf;
int	len,now_len,size;

	size=0;
	buf=NULL;

	now_len=0;
	while(1){
		fgets(data,sizeof(data),fp);
		if(feof(fp)){
			break;
		}
		len=strlen(data);
		if(now_len+len>=size){
			if(now_len+len-size>4096){
				size=now_len+len+1;
			}
			else{
				size+=4096;
			}
			if(buf==NULL){
				buf=(char *)calloc(size,sizeof(char));
			}
			else{
				buf=(char *)realloc(buf,size*sizeof(char));
			}
		}
		strcpy(&buf[now_len],data);
		now_len+=len;
		if(now_len>0&&buf[now_len-1]=='\n'){
			break;
		}
	}

	*ret_buf=buf;

	return(now_len);
}

/* 拡張子からコンテントタイプを求める */
int	GetContentType(char *type,char *ext)
{
HKEY	hKey;
DWORD	size;
BOOL	success;

	//標準MIMEタイプ
	success = FALSE;
	if (stricmp(ext,".html")==0 || stricmp(ext,".html")==0){
		strcpy(type,"text/html"); success=TRUE;
	}else if (stricmp(ext,".jpg")==0 || stricmp(ext,".jpeg")==0){
		strcpy(type,"image/jpeg"); success=TRUE;
	}else if (stricmp(ext,".gif")==0){
		strcpy(type,"image/gif"); success=TRUE;
	}else if (stricmp(ext,".png")==0){
		strcpy(type,"image/png"); success=TRUE;
	}else if (stricmp(ext,".txt")==0 || stricmp(ext,".text")==0){
		strcpy(type,"text/plain"); success=TRUE;
	}else if (stricmp(ext,".sht")==0 || stricmp(ext,".shtml")==0){
		strcpy(type,"text/html"); success=TRUE;
	}else if (stricmp(ext,".map")==0){
		strcpy(type,"text/plain"); success=TRUE;
	}else if (stricmp(ext,".mid")==0){
		strcpy(type,"audio/x-midi"); success=TRUE;
	}else if (stricmp(ext,".mov")==0){
		strcpy(type,"video/quicktime"); success=TRUE;
	}else if (stricmp(ext,".pdf")==0){
		strcpy(type,"application/pdf"); success=TRUE;
	}else if (stricmp(ext,".swf")==0){
		strcpy(type,"application/x-shockwave-flash"); success=TRUE;
	}else if (stricmp(ext,".css")==0){
		strcpy(type,"text/css"); success=TRUE;
	}

	if (!success){
		//レジストリから読み取る
		if (::RegOpenKeyEx(HKEY_CLASSES_ROOT,ext,0,KEY_ALL_ACCESS,&hKey) == ERROR_SUCCESS)
		{
			//読み込み
			size = 200;
			if (ERROR_SUCCESS==::RegQueryValueEx(hKey,"Content Type",0,NULL,(LPBYTE)type,&size)){
				type[size] = '\0';
				success = TRUE;
			}
			::RegCloseKey(hKey);
		}
	}

	if (!success){
		strcpy(type,"application/octet-stream");
	}

	return 0;




}

/* データのデバッグ表示 */
int DebugData(R_S_LINE *r_s_line,R_S_HEAD *r_s_head,R_S_CONTENT *content)
{
int	i;

	LogPrint("<<<<<\n");

	LogPrint("method=%s\n",r_s_line->Method);
	LogPrint("arg=%s\n",r_s_line->Arg);
	if(r_s_line->Version!=NULL){
		LogPrint("version=%s\n",r_s_line->Version);
	}
	else{
		LogPrint("version=NULL\n");
	}

	for(i=0;i<r_s_head->No;i++){
		LogPrint("%s=%s\n",r_s_head->Data[i].Name,r_s_head->Data[i].Val);
	}

	LogPrint("content:size=%d\n",content->Size);
//	if(content->Size!=0){
//		LogPrint("content:data=%s\n",content->Data);
//	}

	LogPrint(">>>>>\n");

	return(0);
}
