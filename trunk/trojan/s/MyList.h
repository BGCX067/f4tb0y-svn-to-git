#if !defined(AFX_MYLIST_H__7FD925BF_AA98_48F6_8CF1_82A6F09C2610__INCLUDED_)
#define AFX_MYLIST_H__7FD925BF_AA98_48F6_8CF1_82A6F09C2610__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyList.h : header file
//
#include  <Afxtempl.h>

struct ItemData
{
public:
	ItemData() : nSubItem( NULL ), nItem( NULL ){}

	//color
	CArray < COLORREF,COLORREF > crText;
	CArray < COLORREF,COLORREF > crBak;

	int nItem;
	int nSubItem;
};
/////////////////////////////////////////////////////////////////////////////
// CMyList window

class CMyList : public CListCtrl
{
// Construction
public:
	CMyList();
	//CMap<DWORD , DWORD& , ItemData * , ItemData *> MapItemColor;
// Attributes
public:

private:
	COLORREF crDefaultTextColor1;
	COLORREF crDefaultBakColor1;

	COLORREF crDefaultTextColor2;
	COLORREF crDefaultBakColor2;

// Operations
public:

public:
	void SetDefaultColor(COLORREF crDefaultTextColor1,COLORREF crDefaultBakColor1,COLORREF crDefaultTextColor2,COLORREF crDefaultBakColor2);
	void SetItemColor(DWORD nItem, int nSubItem, COLORREF clrText, COLORREF clrBkgnd,int nTotalColumn);
	void SetItemColor(DWORD nItem, COLORREF clrText, COLORREF clrBkgnd,int nTotalColumn);
	void SetItemColor(DWORD nItem, int nTotalColumn);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyList();
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	//void SetItemColor(DWORD nItem, COLORREF clrText, COLORREF clrBkgnd);
	// Generated message map functions
protected:
	//{{AFX_MSG(CMyList)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYLIST_H__7FD925BF_AA98_48F6_8CF1_82A6F09C2610__INCLUDED_)
