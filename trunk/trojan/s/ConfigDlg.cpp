// ConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "netcorpseserver.h"
#include "ConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ini.h"

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog


CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg)
	DDX_Control(pDX, IDC_CHECK2, m_cLineOff);
	DDX_Control(pDX, IDC_CHECK1, m_cLineOn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg message handlers

BOOL CConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);	

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

	int m_nPort = ini.GetInt(_T("Settings"), _T("Port"), -1);
	BOOL m_bLineOnWav = ini.GetBool(_T("Settings"), _T("LineOn"), 1);
	BOOL m_bLineOffWav = ini.GetBool(_T("Settings"), _T("LineOff"), 1);

	if(m_nPort <= 0 || m_nPort > 65535)
	{
		m_nPort = 10205;
		m_bLineOnWav = TRUE;
		m_bLineOffWav = TRUE;
		
		ini.WriteInt(_T("Settings"), _T("Port"),m_nPort);
		ini.WriteBool(_T("Settings"), _T("LineOn"),m_bLineOnWav);
		ini.WriteBool(_T("Settings"), _T("LineOff"),m_bLineOffWav);
	}

	CString str;
	str.Format("%d",m_nPort);
	SetDlgItemText(IDC_EDIT1,str);

	
	m_cLineOn.SetCheck(m_bLineOnWav);
	m_cLineOff.SetCheck(m_bLineOffWav);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigDlg::OnOK() 
{
	CString str;
	GetDlgItemText(IDC_EDIT1,str);
	
	int m_nPort = atoi(str);
	
	if(m_nPort <= 0 || m_nPort > 65535)
	{
		AfxMessageBox("请填写正确的端口号");
		return;
	}
	
	BOOL m_bLineOnWav = m_cLineOn.GetCheck();
	BOOL m_bLineOffWav = m_cLineOff.GetCheck();
	
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
	
	ini.WriteInt(_T("Settings"), _T("Port"),m_nPort);
	ini.WriteBool(_T("Settings"), _T("LineOn"),m_bLineOnWav);
	ini.WriteBool(_T("Settings"), _T("LineOff"),m_bLineOffWav);

	AfxMessageBox("应用成功!");

	CDialog::OnOK();	
}
