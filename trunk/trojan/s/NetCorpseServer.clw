; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CNetCorpseServerDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "netcorpseserver.h"
LastPage=0

ClassCount=10
Class1=CNetCorpseServerApp
Class2=CAboutDlg
Class3=CNetCorpseServerDlg
Class4=CResizingDialog

ResourceCount=5
Resource1=IDD_DIALOG4
Resource2=IDD_NETCORPSESERVER_DIALOG
Class5=CConfigDlg
Resource3=IDD_DIALOG1
Class6=CRegDlg
Resource4=IDD_DIALOG3
Class7=CConfigDlg2
Class8=CWelcomeDlg
Class9=CScreenDlg
Class10=CFileManager
Resource5=IDR_MENU_FILEOPT

[CLS:CNetCorpseServerApp]
Type=0
BaseClass=CWinApp
HeaderFile=NetCorpseServer.h
ImplementationFile=NetCorpseServer.cpp
LastObject=CNetCorpseServerApp

[CLS:CAboutDlg]
Type=0
BaseClass=CDialog
HeaderFile=NetCorpseServerDlg.cpp
ImplementationFile=NetCorpseServerDlg.cpp
LastObject=CAboutDlg

[CLS:CNetCorpseServerDlg]
Type=0
BaseClass=CDialog
HeaderFile=NetCorpseServerDlg.h
ImplementationFile=NetCorpseServerDlg.cpp
Filter=D
VirtualFilter=dWC
LastObject=CNetCorpseServerDlg

[CLS:CResizingDialog]
Type=0
BaseClass=CDialog
HeaderFile=ResizingDialog.h
ImplementationFile=ResizingDialog.cpp

[DLG:IDD_NETCORPSESERVER_DIALOG]
Type=1
Class=CNetCorpseServerDlg
ControlCount=27
Control1=IDC_LIST1,SysListView32,1342242825
Control2=IDC_RADIO1,button,1342308361
Control3=IDC_RADIO2,button,1342177289
Control4=IDC_STATIC_INFO4,button,1342177287
Control5=IDC_STATIC_IP,static,1342308352
Control6=IDC_EDIT1,edit,1350631552
Control7=IDC_STATIC_PORT,static,1342308352
Control8=IDC_EDIT2,edit,1350631552
Control9=IDC_ATTACK,button,1342242816
Control10=IDC_STATIC_INFO5,button,1342177287
Control11=IDC_STATIC_URL,static,1342308352
Control12=IDC_EDIT3,edit,1350631552
Control13=IDC_DOWNLOAD,button,1342242816
Control14=IDC_CREATE,button,1342242816
Control15=IDC_LISTEN,button,1342242816
Control16=IDC_SET,button,1342242816
Control17=IDC_ATTACKSTOP,button,1342242816
Control18=IDC_STATIC_STATUS,static,1342308352
Control19=IDC_STATIC,button,1342177287
Control20=IDC_STATIC,button,1342177287
Control21=IDC_RADIO_DOS,button,1342177289
Control22=IDC_RADIO_SELECT,button,1342177289
Control23=IDC_SLIDER1,msctls_trackbar32,1342242840
Control24=IDC_STATIC_ONLINE,static,1342308352
Control25=IDC_STATIC,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_STATIC_LISTENSTATUS,static,1342308352

[DLG:IDD_DIALOG1]
Type=1
Class=CConfigDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342308352
Control3=IDC_EDIT1,edit,1350631552
Control4=IDC_CHECK1,button,1342242819
Control5=IDC_CHECK2,button,1342242819

[CLS:CConfigDlg]
Type=0
HeaderFile=ConfigDlg.h
ImplementationFile=ConfigDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_CHECK1

[CLS:CRegDlg]
Type=0
HeaderFile=RegDlg.h
ImplementationFile=RegDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CRegDlg
VirtualFilter=dWC

[DLG:IDD_DIALOG3]
Type=1
Class=CConfigDlg2
ControlCount=7
Control1=IDC_EDIT1,edit,1350631552
Control2=IDC_EDIT2,edit,1350631552
Control3=IDOK,button,1342242817
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT5,edit,1350631552

[CLS:CConfigDlg2]
Type=0
HeaderFile=ConfigDlg2.h
ImplementationFile=ConfigDlg2.cpp
BaseClass=CDialog
Filter=D
LastObject=CConfigDlg2
VirtualFilter=dWC

[DLG:IDD_DIALOG4]
Type=1
Class=CWelcomeDlg
ControlCount=2
Control1=IDC_PROGRESS1,msctls_progress32,1350565888
Control2=IDC_STATIC,static,1342177294

[CLS:CWelcomeDlg]
Type=0
HeaderFile=WelcomeDlg.h
ImplementationFile=WelcomeDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CWelcomeDlg
VirtualFilter=dWC

[CLS:CScreenDlg]
Type=0
HeaderFile=ScreenDlg1.h
ImplementationFile=ScreenDlg1.cpp
BaseClass=CDialog
Filter=D
LastObject=CScreenDlg
VirtualFilter=dWC

[CLS:CFileManager]
Type=0
HeaderFile=FileManager.h
ImplementationFile=FileManager.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_TREE_DIR
VirtualFilter=dWC

[MNU:IDR_MENU_FILEOPT]
Type=1
Class=?
Command1=ID_MENU_FILEOPT_SAVE
Command2=ID_MENU_FILEOPT_RUN
Command3=ID_MENU_FILEOPT_DELFILE
CommandCount=3

