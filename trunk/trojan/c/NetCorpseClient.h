// NetCorpseClient.h : main header file for the NETCORPSECLIENT application
//

#if !defined(AFX_NETCORPSECLIENT_H__863A5A1A_D898_4ECC_8CC8_A123708F905C__INCLUDED_)
#define AFX_NETCORPSECLIENT_H__863A5A1A_D898_4ECC_8CC8_A123708F905C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseClientApp:
// See NetCorpseClient.cpp for the implementation of this class
//

class CNetCorpseClientApp : public CWinApp
{
public:
	CNetCorpseClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetCorpseClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNetCorpseClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETCORPSECLIENT_H__863A5A1A_D898_4ECC_8CC8_A123708F905C__INCLUDED_)
