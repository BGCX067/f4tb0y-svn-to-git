#if !defined(AFX_CONFIGDLG2_H__A3602434_E90C_4006_9DA0_79FEBE195F58__INCLUDED_)
#define AFX_CONFIGDLG2_H__A3602434_E90C_4006_9DA0_79FEBE195F58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigDlg2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg2 dialog

class CConfigDlg2 : public CDialog
{
// Construction
public:
	CConfigDlg2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigDlg2)
	enum { IDD = IDD_DIALOG3 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigDlg2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigDlg2)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGDLG2_H__A3602434_E90C_4006_9DA0_79FEBE195F58__INCLUDED_)
