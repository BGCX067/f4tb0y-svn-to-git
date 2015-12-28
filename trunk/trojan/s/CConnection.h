#ifndef CCONNECTION_H
#define CCONNECTION_H

#include <winsock.h>
#include <stdio.h>

#pragma comment(lib, "wsock32.lib")		// search for wsock32 lib at compile time
#pragma comment(lib, "mpr.lib")			// search for mpr lib at compile time

typedef void (*CALLBACKFUNC)(DWORD ptr);

//同步控制
class CSync
{
public:
	CSync ();
	~CSync ();

public:
	CSync (CSync& s);
	CSync& operator= (CSync& s);

	void Enter () const;
	void Leave () const;
	
private:
	HANDLE m_sync;
};

//错误信息查询
class CError
{
	long m_number;

	CError (long err);

	friend class CNetworking;
	friend class CConnection;
	friend class CSync;

public:
	long GetErrorString (char* str, long len);
};

//一个FIFO风格的链表
class CConnectionList
{
	friend class CConnection;

	class Node
	{
	public:
		CConnection* m_Con;
		Node*		 m_Next;
		
		Node ();
	};

	Node* m_First; //头指针
	Node* m_Last;  //尾指针
	int	  m_Length;

public:
	CConnectionList ();
	~CConnectionList ();

public:
	//add a connection to the end of the list
	void		 AddTail(CConnection* con);
	//remove a connection from the beginning of the list
	CConnection* RemoveHead();
	//remove a connection at index i
	CConnection* RemoveAt(int i);
	//return a connection at index i
	CConnection* GetItemAt(int i);
	//just use operator [] to get a connection at index i (m_ConList[i]?)
	CConnection* operator [] (int i);
	long		 GetLength();
	//get the index of list by the euqal connection
	long         GetEuqalIndex(CConnection* con);
};

//	class definition for CNetworking
//	this class is thread-safe!
//	access to Tag is NOT synchronized!
class CNetworking
{
private:
	static long		m_count;

	CSync			m_sync;
	CConnectionList	m_accepted;

	SOCKET			m_socket;
	sockaddr_in		m_addr;

	HANDLE			hAcceptEvent;
	CALLBACKFUNC	hAcceptFunc;

	HANDLE			hAcceptThread;
	DWORD			dwAcceptThreadID;

	CError			m_lasterror;

	int AcceptWait ();
	static int AcceptThread (void* pThis);

	void SetLastError (long err);

	//	allow CConnection to do stuff with this class
	//	actually, it should only check if Winsock is
	//	already intialized or not
	friend class CConnection;

public:
	CNetworking ();
	~CNetworking ();

public:
	bool			Listen (int port);

	void			StopListen ();
	CConnection*	GetAccepted ();

	void			SetAcceptEvent (HANDLE hEvent);
	void			SetAcceptFunc (CALLBACKFUNC hFunc);

	int				HasAccepted ();
	bool			IsListening ();

	bool			GetLocalName(char *localname, int len);
	bool			GetLocalIP(char *localip, int len);
	bool			GetLocalIPs(char *localips, int len);
	bool			ResolveName(char *hostip, char *hostname, int len);
	bool			ResolveIP(char *hostname, char *hostip, int len);
	bool			ResolveIPs(char *hostname, char *hostips, int len);

	bool			GetNeighborhood(char* names, int len);

	void			GetLastError (char* str, long len);

	DWORD			Tag;
};

//	class definition for CConnection
//	this class is thread-safe!
//	access to Tag is NOT synchronized!
class CConnection
{
	//	used to cache the received data
	//	this class is thread-safe!
	class CDataStack 
	{
	private:
		char* m_buffer;
		long  m_length;
		CSync m_sync;

	public:
		CDataStack ();
		~CDataStack ();

	public:
		//数据栈尾部追加数据
		void Append(const char* data, int len);
		//数据栈头部移除数据
		int  Remove(char* data, int len);
		//得到数据栈数据长度
		int  GetLength();
	};

	static long		m_count;

	HANDLE			m_event;
	CSync			m_sync;
	CDataStack		m_data;

	SOCKET			m_socket;
	sockaddr_in		m_addr;

	HANDLE			hRecvEvent;
	CALLBACKFUNC	hRecvFunc;
	HANDLE			hCloseEvent;
	CALLBACKFUNC	hCloseFunc;

	HANDLE			hRecvThread;
	DWORD			dwRecvThreadID;

	CError			m_lasterror;

	int RecvWait ();
	static int RecvThread (void* pThis);

	void SetLastError (long err);

	//	allow CNetworking to do stuff with this class
	friend class CNetworking;

public:

	CConnection ();
	CConnection (const char* host, unsigned short port);
	~CConnection ();

	bool Connect (const char* host, unsigned short port);
	void Disconnect ();

	bool PeerInfo (char* host, int host_len, unsigned int* port);

	int	 Send (const char* buffer, int bufferlen);
	int	 Receive (char* buffer, int bufferlen);

	void SetReceiveEvent (HANDLE hEvent);
	void SetReceiveFunc (CALLBACKFUNC hFunc);
	void SetCloseEvent (HANDLE hEvent);
	void SetCloseFunc (CALLBACKFUNC hFunc);

	bool IsConnected ();
	int	 HasReceived ();

	void GetLastError (char* str, long len);

	DWORD Tag;
};

#endif