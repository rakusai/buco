// Buco.h : BUCO アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_BUCO_H__0634CB43_E75A_4235_9C44_835D2643B2C2__INCLUDED_)
#define AFX_BUCO_H__0634CB43_E75A_4235_9C44_835D2643B2C2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CBucoApp:
// このクラスの動作の定義に関しては Buco.cpp ファイルを参照してください。
//

class CBucoApp : public CWinApp
{
public:
	CBucoApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CBucoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CBucoApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_BUCO_H__0634CB43_E75A_4235_9C44_835D2643B2C2__INCLUDED_)
