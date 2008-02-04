; CLW ファイルは MFC ClassWizard の情報を含んでいます。

[General Info]
Version=1
LastClass=CBucoDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Buco.h"

ClassCount=4
Class1=CBucoApp
Class2=CBucoDlg

ResourceCount=3
Resource2=IDR_MAINFRAME
Resource3=IDD_BUCO_DIALOG

[CLS:CBucoApp]
Type=0
HeaderFile=Buco.h
ImplementationFile=Buco.cpp
Filter=N

[CLS:CBucoDlg]
Type=0
HeaderFile=BucoDlg.h
ImplementationFile=BucoDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_BUCO_DIALOG]
Type=1
Class=CBucoDlg
ControlCount=6
Control1=IDC_STATIC,static,1342308352
Control2=IDC_EDIT_NAME,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT_LOG,edit,1342244996
Control5=IDC_CHECK_AUTO_LOGIN,button,1342252035
Control6=IDOK,button,1342242817

