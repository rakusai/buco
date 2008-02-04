// BucoDlg.h : ヘッダー ファイル
//

#if !defined(AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_)
#define AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_

#include "FTPclient.h"	// ClassView によって追加されました。
#include "ImageStone.h"
#include "MyHttp.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBucoDlg ダイアログ

class CBucoDlg : public CDialog
{
// 構築
public:
	void SetStatus(LPCTSTR text);
	BOOL UploadFile(LPCTSTR path);
	BOOL LogoutFromServer();
	BOOL LoginToServer();
	BOOL SendScreenShot();
	BOOL GetScreenShot();
	CBucoDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CBucoDlg)
	enum { IDD = IDD_BUCO_DIALOG };
		// メモ: この位置に ClassWizard によってデータ メンバが追加されます。
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CBucoDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CBucoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CFTPclient m_ftp;
	CMyHttp m_http;
	CString m_strName;
	BOOL m_sending;
	BOOL m_moved;
	BOOL m_starting;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_)
