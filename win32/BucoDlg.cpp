// BucoDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "Buco.h"
#include "BucoDlg.h"
#include "ImageStone.h"
#include "FTPclient.h"

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
// CBucoDlg ダイアログ

CBucoDlg::CBucoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBucoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBucoDlg)
		// メモ: この位置に ClassWizard によってメンバの初期化が追加されます。
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_sending = FALSE;
	m_moved = FALSE;
	m_starting = FALSE;
}

void CBucoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBucoDlg)
		// メモ: この場所には ClassWizard によって DDX と DDV の呼び出しが追加されます。
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBucoDlg, CDialog)
	//{{AFX_MSG_MAP(CBucoDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBucoDlg メッセージ ハンドラ

BOOL CBucoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	m_strName = AfxGetApp()->GetProfileString("Buco","Name","");
	SetDlgItemText(IDC_EDIT_NAME,m_strName);
	if (m_strName != ""){
		((CButton*)GetDlgItem(IDC_CHECK_AUTO_LOGIN))->SetCheck(1);
		CBucoDlg::OnOK() ;
	}

	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CBucoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR CBucoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CBucoDlg::OnTimer(UINT nIDEvent) 
{
	POINT pt;
	::GetCursorPos(&pt);

	static int distance = 0;
	static POINT old_pt = pt;
	static clock_t starttime = clock();
	
	if (pt.x != old_pt.x || pt.y != old_pt.y){
		distance += sqrt(pow(pt.x - old_pt.x,2) + pow(pt.y - old_pt.y,2));

		//カーソル位置が変わった！
		m_moved = TRUE;
		old_pt = pt;
	}


	clock_t difftime = clock() - starttime;

	if (difftime > 300 && m_http.IsSending() == FALSE){
		if (m_moved){
			//速度を集計
			CString t;
			t.Format("OK %d px/sec",distance*1000/difftime,distance,difftime);
			starttime = clock();

			distance = 0;
			//スクリーンショットを取る
			GetScreenShot();
			//転送開始
			SendScreenShot();
			m_moved = FALSE;
			SetStatus("Sending");
		}else{
			CString err;
			if (m_http.GetError(err)){
				SetStatus(err);
			}else{
				SetStatus("Waiting");
			}
		}
	}

	
	// TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	
	CDialog::OnTimer(nIDEvent);
}


BOOL CBucoDlg::GetScreenShot()
{
	//スクリーンショットを取る
	POINT pt;
	::GetCursorPos(&pt);


	CRect rect = CRect(pt.x-58,pt.y-58,pt.x+58,pt.y+58);

	char dir[MAX_PATH]="";
	GetCurrentDirectory(MAX_PATH,dir);
	CString path = dir;
	path += "\\" + m_strName + ".png";

	FCObjImage   img ;
	FCWin32::CaptureScreen (img, rect) ;
	img.ConvertTo24Bit();
//	FCPixelBlur_Box   aCmd (5, true) ;
//	 FCPixelIllusion   aCmd(3) ;
//	FCPixelConvertTo8BitGray   aCmd ;
//FCPixelPosterize   aCmd (2) ;
//	  FCPixelBlur_Zoom   aCmd (15) ;
//FCPixelRipple   aCmd (10, 30) ;

  //  img.SinglePixelProcessProc (aCmd) ;

	img.Save (path) ;


//	char* buffer = new char[5000000];
//	int len = 5000000;
//	len = obj.SaveDesktopAsMemory((char*)buffer , 5000000, JPG_FILE, &rect);
//	obj.SaveDesktopAsMemory((char*)(LPCTSTR)path, JPG_FILE, &rect);
//	obj.SaveDesktopAsFile((char*)(LPCTSTR)path, JPG_FILE, &rect);

	return TRUE;

}


static UINT AFX_CDECL ThreadProcCalc(LPVOID pParam)
{
	CMyHttp *cp=(CMyHttp*)pParam;
	cp->UploadFile();
	return 0;
}

BOOL CBucoDlg::SendScreenShot()
{
	AfxBeginThread(ThreadProcCalc,&m_http);
//	m_http.UploadFile();

//	UploadFile(path);
/*
	// move a file by FTP
	CString lfile = path;
	CString rfile = "/home/rakusai/public_html/buco/" + m_strName + ".jpg";
	BOOL pasv = FALSE;
	BOOL get = FALSE;
//	m_ftp.MoveFile(rfile,lfile,pasv,get);

*/











	// show the result of the operation
	//MessageBox(m_ftp.m_retmsg);

	return TRUE;

}

//DEL void CBucoDlg::OnButton1() 
//DEL {
//DEL 
//DEL 
//DEL 
//DEL 
//DEL 
//DEL 
//DEL }

BOOL CBucoDlg::LoginToServer()
{
	//ログイン

	CString host = "enzin.net";
	int port = 21;
	CString user = "admin";
	CString pass = "2fdcv84nmr";
	CString acct = "";
	CString fwhost = "";
	CString fwuser = "";
	CString fwpass = "";
	int fwport = 21;
	int logontype = 0;


	// connect to FTP server
	if(!m_ftp.LogOnToServer(host,port,user,pass,acct,fwhost,fwuser,fwpass,fwport,logontype)) {
		MessageBox(m_ftp.m_retmsg);
		return FALSE;
	}

	return TRUE;
}

BOOL CBucoDlg::DestroyWindow() 
{
	//ログアウト
//	if (m_starting){
//		LogoutFromServer();
//	}
	return CDialog::DestroyWindow();
}

BOOL CBucoDlg::LogoutFromServer()
{
	// disconnect from server
	m_ftp.LogOffServer();

	return TRUE;
}

void CBucoDlg::OnOK() 
{
	//Start
	if (!m_starting){
		SetTimer(1,30,NULL);
		SetStatus("Connecting...");



		//ログイン
		GetDlgItemText(IDC_EDIT_NAME,m_strName);
//		LoginToServer();
		GetDlgItem(IDC_EDIT_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_AUTO_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDOK)->SetWindowText("Stop");
		m_starting = TRUE;

		BOOL autologin = ((CButton*)GetDlgItem(IDC_CHECK_AUTO_LOGIN))->GetCheck();
		if (autologin){
			AfxGetApp()->WriteProfileString("Buco","Name",m_strName);
		}else{
			AfxGetApp()->WriteProfileString("Buco","Name","");
		}
		char dir[MAX_PATH]="";
		GetCurrentDirectory(MAX_PATH,dir);
		CString path = dir;
		path += "\\" + m_strName + ".png";
		
		m_http.Init(m_strName,path);
	}else{
		SetStatus("");
		KillTimer(1);
		GetDlgItem(IDC_EDIT_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_AUTO_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDOK)->SetWindowText("Start");
//		LogoutFromServer();
		m_starting = FALSE;
	}
	

//	CDialog::OnOK();
}

BOOL CBucoDlg::UploadFile(LPCTSTR path)
{
	//指定ファイルをアップロード
//	CString path = m_sUploadPath;

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
		AfxMessageBox("Can not open the server.");
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
	sprintf(line,"Content-Disposition: form-data; name=\"data\"; filename=\"%s\"\r\n",path);
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Type: application/octet-stream\r\n");
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Transfer-Encoding: binary\r\n\r\n");
	AddContent(&content,line,strlen(line));

	//ファイルを開く
	HANDLE hFile = CreateFile(path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
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
	HttpSendRequest(
		hHttpRequest,
		NULL,
		0,
		content.Data,
		content.Size);

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


	return TRUE;
}

void CBucoDlg::SetStatus(LPCTSTR text)
{
	CString log = text;
//	GetDlgItemText(IDC_EDIT_LOG,log);
	SetDlgItemText(IDC_EDIT_LOG,text);
//	SetWindowText("Buco - " + log);
}
