// NetCorpseServer.h : main header file for the NETCORPSESERVER application
//

#if !defined(AFX_NETCORPSESERVER_H__467FFA57_300F_479F_A5B8_6FE191DA6693__INCLUDED_)
#define AFX_NETCORPSESERVER_H__467FFA57_300F_479F_A5B8_6FE191DA6693__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseServerApp:
// See NetCorpseServer.cpp for the implementation of this class
//

class CNetCorpseServerApp : public CWinApp
{
public:
	CNetCorpseServerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetCorpseServerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNetCorpseServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETCORPSESERVER_H__467FFA57_300F_479F_A5B8_6FE191DA6693__INCLUDED_)
