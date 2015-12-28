// MyList.cpp : implementation file
//

#include "stdafx.h"
#include "MyList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CMyList

CMyList::CMyList()
{
	crDefaultTextColor1 = RGB(0,0,0);
	crDefaultBakColor1 =  RGB(192,192,192);

	crDefaultTextColor2 = RGB(0,0,0);
	crDefaultBakColor2 = RGB(255,255,255);
}

CMyList::~CMyList()
{
}


BEGIN_MESSAGE_MAP(CMyList, CListCtrl)
	//{{AFX_MSG_MAP(CMyList)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
END_MESSAGE_MAP()

void CMyList::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	/*
// TODO: Add your control notification handler code here
	*pResult = CDRF_DODEFAULT;
	NMLVCUSTOMDRAW * lplvdr=(NMLVCUSTOMDRAW*)pNMHDR;
	NMCUSTOMDRAW &nmcd = lplvdr->nmcd;
	
	switch(lplvdr->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
			
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
			
	case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			ItemData *ItemColor;
			if( MapItemColor.Lookup(nmcd.dwItemSpec, ItemColor))
			{
				if(ItemColor->nSubItem == lplvdr->iSubItem)
				{
					lplvdr->clrText = (unsigned long )*ItemColor->crText;
					*pResult = CDRF_DODEFAULT;
				}
			}
		}
		break;


	}
	*/

	//This code based on Michael Dunn's excellent article on
	//list control custom draw at http://www.codeproject.com/listctrl/lvcustomdraw.asp

	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
	}
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        // This is the notification message for an item.  We'll request
        // notifications before each subitem's prepaint stage.
		/*ItemData *ItemColor;
		if( MapItemColor.Lookup(pLVCD->nmcd.dwItemSpec, ItemColor))
		{			
			pLVCD->clrTextBk = ItemColor->crBak;
			pLVCD->clrText = ItemColor->crText;
			*pResult = CDRF_DODEFAULT;			
		}*/

		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		ItemData* p = (ItemData*)(CListCtrl::GetItemData(pLVCD->nmcd.dwItemSpec));
		if(p != NULL)
		{

		ASSERT(p != NULL);
		ASSERT(pLVCD->iSubItem >= 0 && pLVCD->iSubItem < p->crBak.GetSize());
		pLVCD->clrText = p->crText[pLVCD->iSubItem];
		pLVCD->clrTextBk = p->crBak[pLVCD->iSubItem];
		}
		*pResult = CDRF_DODEFAULT;

		/*
        ItemData *ItemColor;
		if( MapItemColor.Lookup(pLVCD->nmcd.dwItemSpec, ItemColor))
		{
			if(ItemColor->nSubItem != -1)
			{
				if(ItemColor->nSubItem == pLVCD->iSubItem)
				{
					pLVCD->clrTextBk = ItemColor->crBak;
					pLVCD->clrText = ItemColor->crText;
					*pResult = CDRF_DODEFAULT;
				}
				else
				{
					//该处还不完善，需要修改 cy.
					pLVCD->clrTextBk = RGB(255,255,255);	
					pLVCD->clrText = RGB(0,0,0);
					*pResult = CDRF_DODEFAULT;
				}
			}
			else
			{
				pLVCD->clrTextBk = ItemColor->crBak;
				pLVCD->clrText = ItemColor->crText;
				*pResult = CDRF_DODEFAULT;	
			}
		}
		*/
	}
}

void CMyList::SetItemColor(DWORD nItem, COLORREF clrText, COLORREF clrBkgnd,int nTotalColumn)
{
	DWORD dword = CListCtrl::GetItemData(nItem);

	ItemData * p = (ItemData*)dword;

	if(p)
	{
		//该项有附加数据，修改或增加
		for(int i = 0; i < nTotalColumn; i++)
		{
			p->crText[i] = clrText;
			p->crBak[i] = clrBkgnd;
		}
		
		CListCtrl::SetItemData(nItem,(DWORD)p);
		this->RedrawItems(nItem, nItem);
		UpdateWindow();
	}
	else
	{
		//该项没附加数据，新增
		ItemData *ItemColor=new ItemData;
		
		ItemColor->crText.SetSize(nTotalColumn);
		ItemColor->crBak.SetSize(nTotalColumn);

		for(int i = 0; i < nTotalColumn; i++)
		{
			ItemColor->crText[i] = clrText;
			ItemColor->crBak[i] = clrBkgnd;
		}		

		CListCtrl::SetItemData(nItem,(DWORD)ItemColor);
		//MapItemColor.SetAt(nItem, ItemColor);
		this->RedrawItems(nItem, nItem);
		UpdateWindow();
	}	
}


void CMyList::SetItemColor(DWORD nItem, int nSubItem, COLORREF clrText, COLORREF clrBkgnd,int nTotalColumn)
{
	DWORD dword = CListCtrl::GetItemData(nItem);

	ItemData * p = (ItemData*)dword;

	if(p)
	{
		//该项有附加数据，修改或增加
		p->crText[nSubItem] = clrText;
		p->crBak[nSubItem] = clrBkgnd;
		
		CListCtrl::SetItemData(nItem,(DWORD)p);
		this->RedrawItems(nItem, nItem);
		UpdateWindow();
	}
	else
	{
		//该项没附加数据，新增
		ItemData *ItemColor=new ItemData;
		
		ItemColor->nSubItem = nSubItem;

		ItemColor->crText.SetSize(nTotalColumn);
		ItemColor->crBak.SetSize(nTotalColumn);

		for(int i = 0; i < nTotalColumn; i++)
		{
			ItemColor->crText[i] = ::GetSysColor(COLOR_WINDOWTEXT);
			ItemColor->crBak[i] = ::GetSysColor(COLOR_WINDOW);
		}

		ItemColor->crText[nSubItem] = clrText;
		ItemColor->crBak[nSubItem] = clrBkgnd;

		CListCtrl::SetItemData(nItem,(DWORD)ItemColor);
		//MapItemColor.SetAt(nItem, ItemColor);
		this->RedrawItems(nItem, nItem);
		UpdateWindow();
	}	
}

void CMyList::SetItemColor(DWORD nItem, int nTotalColumn)
{
	if(nItem % 2)
	{
		SetItemColor(nItem,crDefaultTextColor2,crDefaultBakColor2,nTotalColumn);
	}
	else
	{
		SetItemColor(nItem,crDefaultTextColor1,crDefaultBakColor1,nTotalColumn);
	}
}

void CMyList::SetDefaultColor(COLORREF crDefaultTextColor1,COLORREF crDefaultBakColor1,COLORREF crDefaultTextColor2,COLORREF crDefaultBakColor2)
{
	crDefaultTextColor1 = crDefaultTextColor1;
	crDefaultBakColor1 = crDefaultBakColor1;

	crDefaultTextColor2 = crDefaultTextColor2;
	crDefaultBakColor2 = crDefaultBakColor2;
}



/////////////////////////////////////////////////////////////////////////////
// CMyList message handlers
