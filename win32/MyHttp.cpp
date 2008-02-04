// BucoDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "Buco.h"
#include "MyHttp.h"

#include "http.h"
#include "sock.h"

#include <wininet.h>


#include <comdef.h>
#include <mshtml.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define		DESKTOP_FILE_BMP		"Desktop.bmp"
#define		DESKTOP_FILE_JPG		"Desktop.jpg"
#define		WINDOW_FILE_BMP			"Window.bmp"
#define		WINDOW_FILE_JPG			"Window.jpg"

/////////////////////////////////////////////////////////////////////////////
// CMyHttp ダイアログ

CMyHttp::CMyHttp()
{
	m_sending = FALSE;
}



BOOL CMyHttp::UploadFile()
{
	//指定ファイルをアップロード
//	CString path = m_sUploadPath;
	m_sending = TRUE;

	CString domain = "sharestyle.com";
	int port = 80;
	CString arg = "/~rakusai/buco/buco.php";
	CString url = "http://" + domain + arg ;

	HINTERNET hInternet;
	HINTERNET hHttpSession;
	HINTERNET hHttpRequest;
	char line[1000];
	char szHeader[1000];
	char boundary[] = "---------------------------7d533c243505c0";
	DWORD ReadSize;
	R_S_CONTENT content;

	/* WININET初期化 */
	hInternet = InternetOpen(
		"Mozilla/4.76 [en_jp] (X11; U; SunOS 5.8 sun4u)",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	if (!hInternet){
		m_strErr = "Can not init internet.";
		m_sending = FALSE;
		return FALSE;
	}

	/* サーバへ接続 */
	hHttpSession = InternetConnect(
		hInternet,
		domain,
		port,
		NULL,
		NULL,
		INTERNET_SERVICE_HTTP,
		0,
		0);

/*	char cookie[3000];
	DWORD clen = 3000;
	InternetGetCookie("http://127.0.0.1:10000/test/index.cgi","current_page2",cookie,&clen);
	SetDlgItemText(IDC_EDIT2,cookie);
*/
	if (!hHttpSession){
		InternetCloseHandle(hInternet);
		m_strErr = "Can not open the server.";
		m_sending = FALSE;
		return FALSE;
	}

	/* HTTP要求の作成 */
	hHttpRequest = HttpOpenRequest(
		hHttpSession,
		"POST",
		arg,
		NULL,
		url,
		NULL,
		0,
		0);

	if (!hHttpRequest){
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hInternet);
		m_strErr = "Can not init request.";
		m_sending = FALSE;
		return FALSE;
	}

	/* HTTP要求のヘッダー作成 */
	sprintf(szHeader, "Content-Type: multipart/form-data; boundary=%s\r\n",boundary);
	HttpAddRequestHeaders(hHttpRequest,szHeader,lstrlen(szHeader),HTTP_ADDREQ_FLAG_REPLACE |HTTP_ADDREQ_FLAG_ADD);
	

	/* HTTP要求のコンテンツ作成 */
	InitContent(&content);

	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n","action","regist");
	AddContent(&content,line,strlen(line));

	//現在のユーザー名
	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n","name",m_strName);
	AddContent(&content,line,strlen(line));


	//ファイル
	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"data\"; filename=\"%s\"\r\n",m_strPath);
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Type: application/octet-stream\r\n");
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Transfer-Encoding: binary\r\n\r\n");
	AddContent(&content,line,strlen(line));

	//ファイルを開く
	HANDLE hFile = CreateFile(m_strPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if (hFile != INVALID_HANDLE_VALUE){
		DWORD size = GetFileSize(hFile,NULL);
		char * buf = new char[size+1];
		ReadFile(hFile,buf,size,&ReadSize,NULL);
		buf[ReadSize] = '\0';
		CloseHandle(hFile);

		AddContent(&content,buf,ReadSize);
		delete[] buf;
	}

	strcpy(line,"\r\n");
	AddContent(&content,line,strlen(line));
	sprintf(line,"--%s--\r\n",boundary);
	AddContent(&content,line,strlen(line));

	/* 作成したHTTP要求の発行 */
	BOOL bReq = HttpSendRequest(
		hHttpRequest,
		NULL,
		0,
		content.Data,
		content.Size);

	if (!bReq){
		FreeContent(&content);
		InternetCloseHandle(hHttpRequest);
		InternetCloseHandle(hHttpSession);
		InternetCloseHandle(hInternet);
		m_strErr = "Can not post data.";
		m_sending = FALSE;
		return FALSE;
	}


	/* コンテンツバッファを初期化 */
	FreeContent(&content);
	InitContent(&content);

	/* コンテンツの内容を取得・表示 */
	while (1){
		if (InternetReadFile(hHttpRequest, line, sizeof(line), &ReadSize) && ReadSize > 0)
		{
			AddContent(&content,line,ReadSize);
		}else{
			break;
		}
	}

	//Keep-Aliveかどうか？

/*
	char url[1000];
	char *re1, *re2;
	if (re1 = _tcsstr(content.Data,"FlashVars=\"")){
		if (re2 = _tcsstr(re1+11,"\"")){
			*re2 = '\0';
			strcpy(url,m_CurUrl);
			strcat(url,"link.cgi?");
			strcat(url,re1+11);
//			AfxMessageBox(url);
			if (!m_wndBrowser.WriteFrame(url)){
				AfxMessageBox("ファイルを貼り付けられませんでした。");
			}
		}
	}else if (re1 = _tcsstr(content.Data,"link.swf")){
		if (re2 = _tcsstr(re1,"\"")){
			strncpy(re1,"link.cgi",8);			//link.swfをlink.cgiに変更
			*re2 = '\0';
			strcpy(url,m_CurUrl);
			strcat(url,re1);
//			AfxMessageBox(re1);
			if (!m_wndBrowser.WriteFrame(url)){
				AfxMessageBox("ファイルを貼り付けられませんでした。");
			}
		}
	}else{
		AfxMessageBox("サーバーからエラー返答です。ファイルを貼り付けられませんでした。");
		if (re1 = _tcsstr(content.Data,"エラー")){
			if (re2 = _tcsstr(re1,"<")){
				*re2 = '\0';
				AfxMessageBox(re1);
			}
		}
	}
*/
	/* 後処理 */
	FreeContent(&content);

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hInternet);

	m_strErr = "";
	m_sending = FALSE;

	return TRUE;
}


void CMyHttp::Init(LPCTSTR name, LPCTSTR path)
{
	m_strName = name;
	m_strPath = path;

}

BOOL CMyHttp::IsSending()
{
	return m_sending;
}

BOOL CMyHttp::GetError(CString &err)
{
	err = m_strErr;
	return (m_strErr != "");
}
