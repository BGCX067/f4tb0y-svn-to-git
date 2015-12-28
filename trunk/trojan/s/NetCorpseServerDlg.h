// NetCorpseServerDlg.h : header file
//

#if !defined(AFX_NETCORPSESERVERDLG_H__74DF1D46_9D41_4590_BC31_61D127D457F3__INCLUDED_)
#define AFX_NETCORPSESERVERDLG_H__74DF1D46_9D41_4590_BC31_61D127D457F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyList.h"

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseServerDlg dialog

class CNetCorpseServerDlg : public CDialog
{
// Construction
public:

	CString m_strSN;
	void SendCMD(CString strSendData);
	CString AttackMode(); 

	CString m_sAppPath;
	int m_nPort;
	BOOL m_bLineOnWav;
	BOOL m_bLineOffWav;

	static void SoundPlay(int n);
	CNetCorpseServerDlg(CWnd* pParent = NULL);	// standard constructor

	static void __cdecl ReceiveCallback (DWORD ptr);
	static void __cdecl CloseCallback (DWORD ptr);
	static void __cdecl AcceptCallback (DWORD ptr);

// Dialog Data
	//{{AFX_DATA(CNetCorpseServerDlg)
	enum { IDD = IDD_NETCORPSESERVER_DIALOG };
	CMyList	m_List;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetCorpseServerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CNetCorpseServerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnListen();
	afx_msg void OnSet();
	afx_msg void OnDownload();
	afx_msg void OnAttack();
	afx_msg void OnAttackstop();
	afx_msg void OnCreate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETCORPSESERVERDLG_H__74DF1D46_9D41_4590_BC31_61D127D457F3__INCLUDED_)
