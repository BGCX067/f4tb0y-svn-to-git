// WelcomeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "netcorpseserver.h"
#include "WelcomeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg dialog


CWelcomeDlg::CWelcomeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWelcomeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWelcomeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWelcomeDlg)
	DDX_Control(pDX, IDC_PROGRESS1, m_cProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcomeDlg, CDialog)
	//{{AFX_MSG_MAP(CWelcomeDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg message handlers

void CWelcomeDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CWelcomeDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(0);

	Disp();	
	
	CDialog::OnTimer(nIDEvent);
}

void CWelcomeDlg::Disp()
{
	COLORREF m_clrText1(RGB(28,28,28));
	COLORREF m_clrText2(RGB(0xc0,0xc0,0xc0));

	UpdateData();
	m_cProgress.SetRange(0,10000);
	//m_cProgress.ShowPercent(m_bShowPercent);
	m_cProgress.SetStartColor(m_clrText1);
	m_cProgress.SetEndColor(m_clrText2);
	//m_cProgress.SetBkColor(m_clrBk);
	//m_cProgress.SetTextColor(m_clrText);
	for(int i=0;i<=10000;i++)
	{
		//PeekAndPump();
		m_cProgress.SetPos(i);
	}

	CDialog::OnOK();
}

BOOL CWelcomeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	SetTimer(0,1000,NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
