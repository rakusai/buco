// BucoDlg.cpp : �C���v�������e�[�V���� �t�@�C��
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
// CBucoDlg �_�C�A���O

CBucoDlg::CBucoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBucoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBucoDlg)
		// ����: ���̈ʒu�� ClassWizard �ɂ���ă����o�̏��������ǉ�����܂��B
	//}}AFX_DATA_INIT
	// ����: LoadIcon �� Win32 �� DestroyIcon �̃T�u�V�[�P���X��v�����܂���B
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_sending = FALSE;
	m_moved = FALSE;
	m_starting = FALSE;
}

void CBucoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBucoDlg)
		// ����: ���̏ꏊ�ɂ� ClassWizard �ɂ���� DDX �� DDV �̌Ăяo�����ǉ�����܂��B
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
// CBucoDlg ���b�Z�[�W �n���h��

BOOL CBucoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���̃_�C�A���O�p�̃A�C�R����ݒ肵�܂��B�t���[�����[�N�̓A�v���P�[�V�����̃��C��
	// �E�B���h�E���_�C�A���O�łȂ����͎����I�ɐݒ肵�܂���B
	SetIcon(m_hIcon, TRUE);			// �傫���A�C�R����ݒ�
	SetIcon(m_hIcon, FALSE);		// �������A�C�R����ݒ�
	
	m_strName = AfxGetApp()->GetProfileString("Buco","Name","");
	SetDlgItemText(IDC_EDIT_NAME,m_strName);
	if (m_strName != ""){
		((CButton*)GetDlgItem(IDC_CHECK_AUTO_LOGIN))->SetCheck(1);
		CBucoDlg::OnOK() ;
	}

	return TRUE;  // TRUE ��Ԃ��ƃR���g���[���ɐݒ肵���t�H�[�J�X�͎����܂���B
}

// �����_�C�A���O�{�b�N�X�ɍŏ����{�^����ǉ�����Ȃ�΁A�A�C�R����`�悷��
// �R�[�h���ȉ��ɋL�q����K�v������܂��BMFC �A�v���P�[�V������ document/view
// ���f�����g���Ă���̂ŁA���̏����̓t���[�����[�N�ɂ�莩���I�ɏ�������܂��B

void CBucoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �`��p�̃f�o�C�X �R���e�L�X�g

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// �N���C�A���g�̋�`�̈���̒���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �A�C�R����`�悵�܂��B
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// �V�X�e���́A���[�U�[���ŏ����E�B���h�E���h���b�O���Ă���ԁA
// �J�[�\����\�����邽�߂ɂ������Ăяo���܂��B
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

		//�J�[�\���ʒu���ς�����I
		m_moved = TRUE;
		old_pt = pt;
	}


	clock_t difftime = clock() - starttime;

	if (difftime > 300 && m_http.IsSending() == FALSE){
		if (m_moved){
			//���x���W�v
			CString t;
			t.Format("OK %d px/sec",distance*1000/difftime,distance,difftime);
			starttime = clock();

			distance = 0;
			//�X�N���[���V���b�g�����
			GetScreenShot();
			//�]���J�n
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

	
	// TODO: ���̈ʒu�Ƀ��b�Z�[�W �n���h���p�̃R�[�h��ǉ����邩�܂��̓f�t�H���g�̏������Ăяo���Ă�������
	
	CDialog::OnTimer(nIDEvent);
}


BOOL CBucoDlg::GetScreenShot()
{
	//�X�N���[���V���b�g�����
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
	//���O�C��

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
	//���O�A�E�g
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



		//���O�C��
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
	//�w��t�@�C�����A�b�v���[�h
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

	/* WININET������ */
	hInternet = InternetOpen(
		"Mozilla/4.76 [en_jp] (X11; U; SunOS 5.8 sun4u)",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);

	/* �T�[�o�֐ڑ� */
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

	/* HTTP�v���̍쐬 */
	hHttpRequest = HttpOpenRequest(
		hHttpSession,
		"POST",
		arg,
		NULL,
		url,
		NULL,
		0,
		0);

	/* HTTP�v���̃w�b�_�[�쐬 */
	sprintf(szHeader, "Content-Type: multipart/form-data; boundary=%s\r\n",boundary);
	HttpAddRequestHeaders(hHttpRequest,szHeader,lstrlen(szHeader),HTTP_ADDREQ_FLAG_REPLACE |HTTP_ADDREQ_FLAG_ADD);
	

	/* HTTP�v���̃R���e���c�쐬 */
	InitContent(&content);

	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n","action","regist");
	AddContent(&content,line,strlen(line));

	//���݂̃��[�U�[��
	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n","name",m_strName);
	AddContent(&content,line,strlen(line));


	//�t�@�C��
	sprintf(line,"--%s\r\n",boundary);
	AddContent(&content,line,strlen(line));
	sprintf(line,"Content-Disposition: form-data; name=\"data\"; filename=\"%s\"\r\n",path);
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Type: application/octet-stream\r\n");
	AddContent(&content,line,strlen(line));
	strcpy(line,"Content-Transfer-Encoding: binary\r\n\r\n");
	AddContent(&content,line,strlen(line));

	//�t�@�C�����J��
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

	/* �쐬����HTTP�v���̔��s */
	HttpSendRequest(
		hHttpRequest,
		NULL,
		0,
		content.Data,
		content.Size);

	/* �R���e���c�o�b�t�@�������� */
	FreeContent(&content);
	InitContent(&content);

	/* �R���e���c�̓��e���擾�E�\�� */
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
				AfxMessageBox("�t�@�C����\��t�����܂���ł����B");
			}
		}
	}else if (re1 = _tcsstr(content.Data,"link.swf")){
		if (re2 = _tcsstr(re1,"\"")){
			strncpy(re1,"link.cgi",8);			//link.swf��link.cgi�ɕύX
			*re2 = '\0';
			strcpy(url,m_CurUrl);
			strcat(url,re1);
//			AfxMessageBox(re1);
			if (!m_wndBrowser.WriteFrame(url)){
				AfxMessageBox("�t�@�C����\��t�����܂���ł����B");
			}
		}
	}else{
		AfxMessageBox("�T�[�o�[����G���[�ԓ��ł��B�t�@�C����\��t�����܂���ł����B");
		if (re1 = _tcsstr(content.Data,"�G���[")){
			if (re2 = _tcsstr(re1,"<")){
				*re2 = '\0';
				AfxMessageBox(re1);
			}
		}
	}
*/
	/* �㏈�� */
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
