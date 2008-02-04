// BucoDlg.h : �w�b�_�[ �t�@�C��
//

#if !defined(AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_)
#define AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_

#include "FTPclient.h"	// ClassView �ɂ���Ēǉ�����܂����B
#include "ImageStone.h"
#include "MyHttp.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBucoDlg �_�C�A���O

class CBucoDlg : public CDialog
{
// �\�z
public:
	void SetStatus(LPCTSTR text);
	BOOL UploadFile(LPCTSTR path);
	BOOL LogoutFromServer();
	BOOL LoginToServer();
	BOOL SendScreenShot();
	BOOL GetScreenShot();
	CBucoDlg(CWnd* pParent = NULL);	// �W���̃R���X�g���N�^

// �_�C�A���O �f�[�^
	//{{AFX_DATA(CBucoDlg)
	enum { IDD = IDD_BUCO_DIALOG };
		// ����: ���̈ʒu�� ClassWizard �ɂ���ăf�[�^ �����o���ǉ�����܂��B
	//}}AFX_DATA

	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B
	//{{AFX_VIRTUAL(CBucoDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �̃T�|�[�g
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:
	HICON m_hIcon;

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
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
// Microsoft Visual C++ �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_BUCODLG_H__7B9E08A7_359E_48F3_A398_1647128F9D5F__INCLUDED_)
