// MyHttp.h : �w�b�_�[ �t�@�C��
//


/////////////////////////////////////////////////////////////////////////////
// MyHttp

class CMyHttp
{
// �\�z
public:
	BOOL GetError(CString& err);
	BOOL IsSending();
	void Init(LPCTSTR name, LPCTSTR path);
	CMyHttp();
	BOOL UploadFile();

private:
	CString m_strErr;
	BOOL m_sending;
	CString m_strName;
	CString m_strPath;
};

