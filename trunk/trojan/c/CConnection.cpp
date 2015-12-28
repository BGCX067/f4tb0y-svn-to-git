#include "StdAfx.h"
#include "CConnection.h"

#define	BACKLOG		10
#define TIMEOUT		10

long CNetworking::m_count = 0;

CNetworking::CNetworking () : m_lasterror (0)
{
	if (m_count == 0 && CConnection::m_count == 0)
	{
		WSADATA	wsd;
		int		err;

		err = WSAStartup (MAKEWORD (1, 1), &wsd);

		if (err != 0)
			throw CError (err);
	}

	hAcceptEvent = NULL;
	hAcceptFunc = NULL;

	dwAcceptThreadID = 0;
	hAcceptThread = NULL;

	m_socket = INVALID_SOCKET;

	m_count ++;
}

CNetworking::~CNetworking ()
{
	StopListen ();

	if (m_count == 1 && CConnection::m_count == 0)
		WSACleanup();

	m_count --;
}

void CNetworking::SetLastError (long err)
{
	m_sync.Enter ();
	m_lasterror.m_number = err;
	m_sync.Leave ();
}

void CNetworking::GetLastError (char* str, long len)
{
	m_sync.Enter ();
	m_lasterror.GetErrorString (str, len);
	m_sync.Leave ();
}

bool CNetworking::Listen (int port)
{
	if (IsListening ())
		StopListen ();

	sockaddr_in	addr;
	SOCKET		sock;

	sock = socket(AF_INET, SOCK_STREAM, 0);			// Create socket

	addr.sin_family = AF_INET;						// Address family Internet
	addr.sin_port = htons (port);					// Assign port 'port' to this socket
	addr.sin_addr.s_addr = htonl (INADDR_ANY);		// No destination

	if (bind (sock, (LPSOCKADDR) &addr, sizeof (addr)) == SOCKET_ERROR)
	{
		closesocket (sock);
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	if (listen (sock, BACKLOG) == SOCKET_ERROR)
	{
		closesocket (sock);
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	m_sync.Enter ();
	m_socket = sock;
	m_addr = addr;
	m_sync.Leave ();

	hAcceptThread = CreateThread (NULL,
								  0,
								  (LPTHREAD_START_ROUTINE) AcceptThread,
								  this,
								  0,
								  (LPDWORD) &dwAcceptThreadID);
	if (!hAcceptThread)
	{
		StopListen ();
		return false;
	}

	return true;
}

void CNetworking::StopListen ()
{
	if (!IsListening ())
	{
		CNetworking::SetLastError (WSAENOTCONN);
		return;
	}

	m_sync.Enter ();
	closesocket (m_socket);
	m_socket = INVALID_SOCKET;
	m_sync.Leave ();

	WaitForSingleObject (hAcceptThread, INFINITE);
}

void CNetworking::SetAcceptFunc (CALLBACKFUNC hFunc)
{
	m_sync.Enter ();
	hAcceptFunc = hFunc;
	m_sync.Leave ();
}

void CNetworking::SetAcceptEvent (HANDLE hEvent)
{
	m_sync.Enter ();
	hAcceptEvent = hEvent;
	m_sync.Leave ();
}

CConnection* CNetworking::GetAccepted()
{
	CConnection* cAccepted;

	m_sync.Enter ();
	cAccepted = m_accepted.RemoveHead();
	m_sync.Leave ();

	return cAccepted;
}

int CNetworking::HasAccepted ()
{
	int iAccepted;

	m_sync.Enter ();
	iAccepted = m_accepted.GetLength ();
	m_sync.Leave ();

	return iAccepted;
}

bool CNetworking::IsListening ()
{
	bool bListening;

	m_sync.Enter ();
	bListening = m_socket != INVALID_SOCKET;
	m_sync.Leave ();

	return bListening;
}

int CNetworking::AcceptWait ()
{
	int			addr_size = sizeof (sockaddr_in);

	SOCKET		lsock;
	SOCKET		csock = INVALID_SOCKET;
	sockaddr_in	caddr = { 0 };

	m_sync.Enter ();
	lsock = m_socket;
	m_sync.Leave ();

	while (IsListening ())
	{
		m_sync.Enter ();
		bool bRightSock = (lsock == m_socket);
		m_sync.Leave ();
		if (!bRightSock) return 1;

		csock = accept (lsock, (LPSOCKADDR) &caddr, &addr_size); // accept a connection
		if (csock == INVALID_SOCKET)
		{
			int aErr = WSAGetLastError();
			if (aErr == WSAEINTR)
			{
				StopListen ();
				return 1;
			}
			else
			{
				//	error: could not accept
				long err = WSAGetLastError ();
				CNetworking::SetLastError (err);

				StopListen ();
				return 1;
			}
		}
		else
		{
			CConnection* cAccepted = new CConnection ();
			cAccepted->m_socket = csock;
			cAccepted->m_addr = caddr;

			cAccepted->hRecvThread = CreateThread (NULL,
												   0,
												   (LPTHREAD_START_ROUTINE) cAccepted->RecvThread,
												   cAccepted,
												   0,
												   (LPDWORD) &cAccepted->dwRecvThreadID);

			if (cAccepted->hRecvThread == NULL)
			{
				cAccepted->Disconnect ();
				CNetworking::SetLastError (1002);
			}
			else
			{
				m_sync.Enter ();
				m_accepted.AddTail(cAccepted);

				HANDLE hEvent = hAcceptEvent;
				CALLBACKFUNC fncCallback = hAcceptFunc;
				m_sync.Leave ();

				if (hEvent) SetEvent (hEvent);				//	if an event was given, pulse the event
				if (fncCallback) (fncCallback) ((DWORD) this);	//	if a function ptr was given, call the function
			}
		}
	}

	return 0;
}

int CNetworking::AcceptThread (void* pThis)
{
	return ((CNetworking*) (pThis))->AcceptWait ();
}

bool CNetworking::GetNeighborhood(char* names, int len)
{
	DWORD	status;
	HANDLE	hEnum;
	DWORD	cbBuffer = 16384;
	DWORD	cEntries = 0xffffffff;
	LPNETRESOURCE lpnr;
	DWORD	namesLast = 0;
	char	nms[2048] = "";

	status = WNetOpenEnum (RESOURCE_CONTEXT, RESOURCETYPE_ANY, 0, NULL, &hEnum);
	if (status != NO_ERROR)
		return false;

	do
	{
		lpnr = (LPNETRESOURCE) GlobalAlloc (GPTR, cbBuffer);
		status = WNetEnumResource (hEnum, &cEntries, lpnr, &cbBuffer);

		if (status == NO_ERROR)
		{
			for(DWORD i = 0; i < cEntries; i++)
			{
				if (lpnr[i].lpRemoteName != NULL)
				{	// it is a computer in the network...
					if (lpnr[i].lpRemoteName[0] == '\\' && lpnr[i].lpRemoteName[1] == '\\')
					{	// recheck, if it really is a computer (because of the "\\")
						int rname_len = strlen (lpnr[i].lpRemoteName);

						for (int j = 2; j < rname_len; j++)
							nms[namesLast + j - 2] = lpnr[i].lpRemoteName[j];

						namesLast += (rname_len - 2);
						nms[namesLast] = 13;
						namesLast++;
					}
				}
			}
		}
		else if (status != ERROR_NO_MORE_ITEMS )
		{
			GlobalFree ((HGLOBAL) lpnr);
			WNetCloseEnum (hEnum);
			return false;
		}

		GlobalFree ((HGLOBAL) lpnr);
	}
	while (status != ERROR_NO_MORE_ITEMS);
	WNetCloseEnum (hEnum);

	memset (names, 0, len);

	int uselen = 2048;
	if (len < uselen) uselen = len;

	memcpy (names, nms, uselen);

	return true;
}

bool CNetworking::GetLocalName(char *localname, int len)
{
	char locname[512] = "";

	int status = gethostname (locname, 512);

	if (status != 0)
	{
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	memset (localname, 0, len);

	int uselen = strlen (locname) + 1;
	if (uselen > len) uselen = len;

	memcpy (localname, locname, uselen);

	return true;
}

bool CNetworking::GetLocalIP(char *localip, int len)
{
	char locname[512];
	char locip[16];

	if (!(GetLocalName(locname, 512) && ResolveIP(locname, locip, 16)))
		return false;
	// not raising an error, bcause GetLocalName or ResolveIP already did it

	memset (localip, 0, len);

	int uselen = strlen (locip) + 1;
	if (len < uselen) uselen = len;

	memcpy (localip, locip, uselen);

	return true;
}

bool CNetworking::GetLocalIPs(char *localips, int len)
{
	char locname[512] = "";
	char locips[256] = "";

	if (!GetLocalName(locname, 512) || !ResolveIPs(locname, locips, 256))
		return false;

	memset (localips, 0, len);

	int uselen = strlen (locips) + 1;
	if (len < uselen) uselen = len;

	memcpy (localips, locips, uselen);

	return true;
}

bool CNetworking::ResolveName(char *hostip, char *hostname, int len)
{
	unsigned long hstaddr = inet_addr (hostip);
	hostent* phe = gethostbyaddr ((char*) &hstaddr, sizeof (unsigned long), AF_INET);

	if (phe == NULL)
	{
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	char *hstname = (char*) phe->h_name;

	memset (hostname, 0, len);

	int uselen = strlen (hstname) + 1;
	if (len < uselen) uselen = len;

	memcpy (hostname, hstname, uselen);

	return true;
}

bool CNetworking::ResolveIP(char *hostname, char *hostip, int len)
{
	hostent* phe = gethostbyname (hostname);

	if (phe == NULL)
	{
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	char hstip[15] = "";
	if (phe->h_addr_list[0] != NULL)
	{
		sprintf (hstip, "%u.%u.%u.%u", 
			(unsigned char) phe->h_addr_list[0][0],
			(unsigned char) phe->h_addr_list[0][1],
			(unsigned char) phe->h_addr_list[0][2],
			(unsigned char) phe->h_addr_list[0][3]);
	}
	else
	{
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}

	memset (hostip, 0, len);

	int uselen = strlen (hstip) + 1;
	if (len < uselen) uselen = len;

	memcpy (hostip, hstip, uselen);

	return true;
}

bool CNetworking::ResolveIPs(char *hostname, char *hostips, int len)
{
	hostent *phe = gethostbyname (hostname);
	char	allips[256], null_ip[256];

	if (phe == NULL || phe->h_addr_list[0] == NULL)
	{
		long err = WSAGetLastError ();
		CNetworking::SetLastError (err);
		return false;
	}
	
	int count = 0,
		i = 0;

	for (i = 0; i < 256; i++)
	{
		allips[i] = 0;
		null_ip[i] = 0;
	}

	while (phe->h_addr_list[count] != NULL && count * 15 < 240)
	{
		char oneip[15] = "";
		for (i = 0; i < 15; i++)
			oneip[i] = 0;

		sprintf(oneip, "%u.%u.%u.%u", 
			(unsigned char) phe->h_addr_list[count][0],
			(unsigned char) phe->h_addr_list[count][1],
			(unsigned char) phe->h_addr_list[count][2],
			(unsigned char) phe->h_addr_list[count][3]);

		for (i = (count * 15); i < (count * 15 + 15); i++)
			null_ip[i] = oneip[i % 15];

		count++;
	}

	int lEnd = 0;
	for (i = 0; i < 15; i++)
	{
		if (null_ip[i * 15] == 0) break;
		for (int j = 0; j < 15; j++)
			allips[lEnd + j] = null_ip[i * 15 + j];

		lEnd = strlen(allips);
		allips[lEnd] = 13;
		lEnd ++;
	}

	for (i = 0; i < len; i++)
		hostips[i] = 0;

	int uselen = strlen(allips);
	if (uselen > len) uselen = len;

	for (i = 0; i < uselen; i++)
		hostips[i] = allips[i];
	hostips[uselen] = 0;

	return true;
}







long CConnection::m_count = 0;

CConnection::CConnection () : m_lasterror (0)
{
	if (m_count == 0 && CNetworking::m_count == 0)
	{
		WSADATA	wsd;
		int		err;

		err = WSAStartup (MAKEWORD (1, 1), &wsd);

		if (err != 0)
			throw CError (err);
	}

	m_lasterror.m_number = 0;

	m_event = CreateEvent (NULL, TRUE, FALSE, NULL);
	hCloseEvent = NULL;
	hCloseFunc = NULL;
	hRecvEvent = NULL;
	hRecvFunc = NULL;

	dwRecvThreadID = 0;
	hRecvThread = NULL;

	m_socket = INVALID_SOCKET;

	m_count ++;
}

CConnection::CConnection (const char* host, unsigned short port) : m_lasterror (0)
{
	if (m_count == 0 && CNetworking::m_count == 0)
	{
		WSADATA	wsd;
		int		err;

		err = WSAStartup (MAKEWORD (1, 1), &wsd);

		if (err != 0)
			throw CError (err);
	}

	m_event = CreateEvent (NULL, TRUE, FALSE, NULL);
	hCloseEvent = NULL;
	hCloseFunc = NULL;
	hRecvEvent = NULL;
	hRecvFunc = NULL;

	dwRecvThreadID = 0;
	hRecvThread = NULL;

	m_socket = INVALID_SOCKET;

	m_count ++;

	if (!Connect (host, port))
	{
		long err = WSAGetLastError ();

		if (err == 0)
			err = m_lasterror.m_number;

		throw CError (err);
	}
}

CConnection::~CConnection ()
{
	Disconnect ();

	if (m_count == 1 && CNetworking::m_count == 0)
		WSACleanup();

	CloseHandle (m_event);
	m_count --;
}

void CConnection::SetLastError (long err)
{
	m_sync.Enter ();
	m_lasterror.m_number = err;
	m_sync.Leave ();
}

void CConnection::GetLastError (char* str, long len)
{
	m_sync.Enter ();
	m_lasterror.GetErrorString (str, len);
	m_sync.Leave ();
}

bool CConnection::Connect (const char* host, unsigned short port)
{
	Disconnect ();

	sockaddr_in	addr;
	SOCKET		sock;

	sock = socket (AF_INET, SOCK_STREAM, 0);	// create socket
	addr.sin_family = AF_INET;					// address family Internet
	addr.sin_port = htons (port);				// set server’s port number
	addr.sin_addr.s_addr = inet_addr (host);	// set server’s IP

	if (connect (sock, (LPSOCKADDR) &addr, sizeof (addr)) == SOCKET_ERROR)
	{
		long err = WSAGetLastError ();
		CConnection::SetLastError (err);
		return false;
	}

	linger lg;
	lg.l_onoff = 1;
	lg.l_linger = 10;
	setsockopt (sock, SOL_SOCKET, SO_LINGER, reinterpret_cast <const char*> (&lg), sizeof (linger));

	hRecvThread = CreateThread (NULL,
								0,
								(LPTHREAD_START_ROUTINE) RecvThread,
								this,
								0,
								(LPDWORD) &dwRecvThreadID);

	if (hRecvThread == NULL)
	{
		CConnection::SetLastError (1002);
		closesocket (sock);
		return false;
	}

	m_sync.Enter ();
	m_socket = sock;
	m_addr = addr;
	m_sync.Leave ();

	return true;
}

void CConnection::Disconnect ()
{
	if (!IsConnected ())
	{
		CConnection::SetLastError (WSAENOTCONN);
		return;
	}

	int		status;
	fd_set	readfds;
	timeval	timeout;

	//	set the event to non-signaled
	ResetEvent (m_event);

	m_sync.Enter ();
	SOCKET s = m_socket;
	m_socket = INVALID_SOCKET;
	m_sync.Leave ();

/*
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
*/
	shutdown (s, 0x01);

	FD_ZERO (&readfds);
	FD_SET (s, &readfds);

	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	status = select (1, &readfds, NULL, NULL, &timeout);
	if (status == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		CConnection::SetLastError (err);
	}

	//	now wait until receive-thread is done (5 seconds max!)
	Sleep (10);
	WaitForSingleObject (hRecvThread, 5000);
	closesocket (s);

	DWORD dwExitCode;
	GetExitCodeThread (hRecvThread, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
	{
		//	wait for the event to be set and then reset
		WaitForSingleObject (m_event, 2000);
		ResetEvent (m_event);

		//	if still active, we might have a dead-lock
		//	might be caused by the callback fct
		TerminateThread (hRecvThread, 0);
	}
	hRecvThread = NULL;
}

bool CConnection::PeerInfo (char* host, int host_len, unsigned int* port)
{
	if (!IsConnected ())
	{
		CConnection::SetLastError (WSAENOTCONN);
		return false;
	}

	sockaddr_in addr;

	m_sync.Enter ();
	addr = m_addr;
	m_sync.Leave ();

	if (port != NULL)
		*port = (unsigned int) ntohs (addr.sin_port);

	if (host != NULL)
	{
		char* ip = inet_ntoa (addr.sin_addr);

		memset (host, 0, host_len);

		int uselen = strlen (ip) + 1;
		if (host_len < uselen) uselen = host_len;

		memcpy (host, ip, host_len);
	}
	return true;
}

int CConnection::Send (const char* buffer, int bufferlen)
{
	if (!IsConnected ())
	{
		CConnection::SetLastError (WSAENOTCONN);
		return 0;
	}

	m_sync.Enter ();
	SOCKET sock = m_socket;
	m_sync.Leave ();

	return send (sock, buffer, bufferlen, 0);
}

int CConnection::Receive (char* buffer, int bufferlen)
{
	int iRecvd;

	m_sync.Enter ();
	iRecvd = m_data.Remove (buffer, bufferlen);
	m_sync.Leave ();

	return iRecvd;
}

void CConnection::SetCloseFunc (CALLBACKFUNC hFunc)
{
	m_sync.Enter ();
	hCloseFunc = hFunc;
	m_sync.Leave ();
}

void CConnection::SetCloseEvent (HANDLE hEvent)
{
	m_sync.Enter ();
	hCloseEvent = hEvent;
	m_sync.Leave ();
}

void CConnection::SetReceiveFunc (CALLBACKFUNC hFunc)
{
	m_sync.Enter ();
	hRecvFunc = hFunc;
	m_sync.Leave ();
}

void CConnection::SetReceiveEvent (HANDLE hEvent)
{
	m_sync.Enter ();
	hRecvEvent = hEvent;
	m_sync.Leave ();
}

bool CConnection::IsConnected ()
{
	bool bConnd;

	m_sync.Enter ();
	bConnd = m_socket != INVALID_SOCKET;
	m_sync.Leave ();

	return bConnd;
}

int CConnection::HasReceived ()
{
	int iRecvd;

	m_sync.Enter ();
	iRecvd = m_data.GetLength();
	m_sync.Leave ();

	return iRecvd;
}

int CConnection::RecvWait ()
{
	const int MSG_CHUNK = 512;

	int		status = 0;
	char	buffer[MSG_CHUNK];
	SOCKET	sock;

	while (true)
	{
		m_sync.Enter ();
		sock = m_socket;
		m_sync.Leave ();

		if (sock == INVALID_SOCKET)
		{
			SetEvent (m_event);
			return 0;
		}

		//	By default sockets are created in blocking mode.
		//	Just keep reading until process destroyed.
		memset (buffer, 0, MSG_CHUNK);

		status = recv (sock, buffer, MSG_CHUNK, 0);
		if (status == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			CConnection::SetLastError (err);

			if (hCloseEvent) SetEvent (hCloseEvent);		//	if an event was given, pulse the event
			if (hCloseFunc) (hCloseFunc) ((DWORD) this);	//	if a function ptr was given, call the function

			Disconnect ();
			SetEvent (m_event);
			return 0;
		}

		if (status > 0)
		{
			m_sync.Enter ();
			m_data.Append (buffer, status);

			HANDLE hEvent = hRecvEvent;
			CALLBACKFUNC fncCallback = hRecvFunc;
			m_sync.Leave ();

			if (hEvent) SetEvent (hEvent);				//	if an event was given, pulse the event
			if (fncCallback) (fncCallback) ((DWORD) this);	//	if a function ptr was given, call the function
		}
		else
		{
			//	socket was "gracefully" closed - thus, no error
			if (hCloseEvent) SetEvent (hCloseEvent);		//	if an event was given, pulse the event
			if (hCloseFunc) (hCloseFunc) ((DWORD) this);	//	if a function ptr was given, call the function

			Disconnect ();
			SetEvent (m_event);
			return 0;
/*
			m_sync.Enter ();
			SOCKET s = m_socket;
			m_socket = INVALID_SOCKET;
			hRecvThread = NULL;
			m_sync.Leave ();

			#define SD_RECEIVE      0x00
			#define SD_SEND         0x01
			#define SD_BOTH         0x02

			//	be nice and shut the socket gracefully
			shutdown (s, 0x01);
			closesocket (s);
*/
		}
	}

	return 0;
}

int CConnection::RecvThread (void* pThis)
{
	return ((CConnection*) (pThis))->RecvWait ();
}

//------------CConnectionList-----------------------------
CConnectionList::CConnectionList ()
{
	m_First = NULL;
	m_Last = NULL;
	m_Length = 0;
}

CConnectionList::~CConnectionList ()
{
	if (m_Length == 0) 
		return;

	Node* next = m_First->m_Next;
	do
	{
		delete m_First;
		m_First = next;

		if (m_First)
			next = m_First->m_Next;
	}while (m_First);
}

//	add a connection to the end of the list
void CConnectionList::AddTail(CConnection* con)
{
	if (m_Length == 0)
	{
		m_Last = m_First = new Node ();
	}
	else
	{
		m_Last->m_Next = new Node ();
		m_Last = m_Last->m_Next;
	}
	m_Last->m_Con = con;

	m_Length ++;
}

//	remove a connection from the beginning of the list
CConnection* CConnectionList::RemoveHead()
{
	if (m_Length == 0) 
		return NULL;

	CConnection* ret = m_First->m_Con;
	Node* newfirst = m_First->m_Next;

	delete m_First;
	m_First = newfirst;

	m_Length --;

	return ret;
}

//	remove a connection at index i
CConnection* CConnectionList::RemoveAt(int i)
{
	if (m_Length == 0) 
		return NULL;
	if (i == 0)
		return RemoveHead();

	Node* ret = m_First;
	for (int j = 0; j < i - 1; j++)
		ret = ret->m_Next;

	Node* d = ret->m_Next;
	CConnection* cret = d->m_Con;
	ret->m_Next = d->m_Next;

	delete d;
	m_Length --;

	return cret;
}

CConnection* CConnectionList::GetItemAt(int i)
{
	return operator[] (i);
}

CConnection* CConnectionList::operator [](int i)
{
	if (m_Length == 0) 
		return NULL;

	Node* ret = m_First;

	for (int j = 0; j < i; j++)
		ret = ret->m_Next;

	return ret->m_Con;
}

long CConnectionList::GetLength ()
{
	return m_Length;
}

long CConnectionList::GetEuqalIndex(CConnection* con)
{
	Node* ret = m_First;
	if(ret->m_Con == con)
	{
		return 0;
	}

	for (int j = 0; j < m_Length - 1; j++)
	{
		ret = ret->m_Next;
		if(ret->m_Con == con)
		{
			return j + 1;
		}
	}

	return -1;
}

CConnectionList::Node::Node ()
{
	m_Con = NULL;
	m_Next = NULL;
}

//--------------------------------------------
CConnection::CDataStack::CDataStack ()
{
	m_buffer = NULL;
	m_length = 0;
}

CConnection::CDataStack::~CDataStack ()
{
	delete[] m_buffer;
}

void CConnection::CDataStack::Append(const char* data, int len)
{
	if (data == NULL || len <= 0) 
		return;

	m_sync.Enter();

	if (m_length == 0)
	{
		m_buffer = new char[len];
		memcpy(m_buffer, data, len);
		m_length = len;
	}
	else
	{
		//	backup old buffer (temporarily)
		char* oldbuff = new char[m_length];
		memcpy(oldbuff, m_buffer, m_length);

		//	create a new buffer that holds the old one PLUS the new stuff
		delete[] m_buffer;
		m_buffer = new char[m_length + len];

		//	copy old buffer and new data into the new (bigger) buffer
		memcpy(m_buffer, oldbuff, m_length);
		memcpy(&m_buffer[m_length], data, len);

		//	store the new length of the data
		m_length += len;

		//	do clean-up
		delete[] oldbuff;
	}
	
	m_sync.Leave();
}

int CConnection::CDataStack::Remove (char* data, int len)
{
	memset(data, 0, len);

	m_sync.Enter();

	//	if the buffer is empty, don't remove anything
	if (m_length == 0)
	{
		m_sync.Leave ();
		return 0;
	}

	//	determine how much we will actually copy
	int remlen = len;
	if (remlen > m_length)
		remlen = m_length;

	//	backup old buffer (temporarily)
	char* oldbuff = new char[m_length];
	memcpy(oldbuff, m_buffer, m_length);

	//	copy part of buffer (or all) to 'data'
	memcpy(data, oldbuff, remlen);

	//	create a new buffer that holds the old one MINUS the removed stuff
	delete[] m_buffer;
	if (m_length - remlen <= 0)
	{
		m_buffer = NULL;
		m_length = 0;
	}
	else
	{
		m_buffer = new char[m_length - remlen];

		//	remove the part that has been copied from the buffer
		memcpy(m_buffer, &oldbuff[remlen], m_length - remlen);

		//	store the new length of the data
		m_length -= remlen;
	}

	//	do clean-up
	delete[] oldbuff;

	m_sync.Leave();

	return remlen;
}

int CConnection::CDataStack::GetLength()
{
	long length;

	m_sync.Enter();

	length = m_length;

	m_sync.Leave();

	return length;
}

//-----------------CSync----------------------------------
CSync::CSync()
{
	m_sync = CreateMutex (NULL, false, NULL);
	if (m_sync == NULL)
		throw CError (1001);
}

CSync::~CSync()
{
	if (m_sync != NULL)
	{
		CloseHandle (m_sync);
		m_sync = NULL;
	}
}

void CSync::Enter () const
{
	WaitForSingleObject (m_sync, INFINITE);
}

void CSync::Leave () const
{
	ReleaseMutex (m_sync);
}

//----------------CError-----------------------------
CError::CError (long err)
{
	m_number = err;
}

long CError::GetErrorString (char* str, long len)
{
	static const long lErrCodes[] = 
		{
			WSAEACCES, 
			WSAEADDRINUSE, 
			WSAEADDRNOTAVAIL, 
			WSAEAFNOSUPPORT, 
			WSAEALREADY, 
			WSAECONNABORTED, 
			WSAECONNREFUSED, 
			WSAECONNRESET, 
			WSAEDESTADDRREQ, 
			WSAEFAULT, 
			WSAEHOSTDOWN, 
			WSAEHOSTUNREACH, 
			WSAEINPROGRESS, 
			WSAEINTR, 
			WSAEINVAL, 
			WSAEISCONN, 
			WSAEMFILE, 
			WSAEMSGSIZE, 
			WSAENETDOWN, 
			WSAENETRESET, 
			WSAENETUNREACH, 
			WSAENOBUFS, 
			WSAENOPROTOOPT, 
			WSAENOTCONN, 
			WSAENOTSOCK, 
			WSAEOPNOTSUPP, 
			WSAEPFNOSUPPORT, 
			WSAEPROCLIM, 
			WSAEPROTONOSUPPORT, 
			WSAEPROTOTYPE, 
			WSAESHUTDOWN, 
			WSAESOCKTNOSUPPORT, 
			WSAETIMEDOUT, 
			WSAEWOULDBLOCK, 
			WSAHOST_NOT_FOUND, 
			WSANOTINITIALISED, 
			WSANO_DATA, 
			WSANO_RECOVERY, 
			WSASYSNOTREADY, 
			WSATRY_AGAIN, 
			WSAVERNOTSUPPORTED, 
			WSAEDISCON, 
			1001, 
			1002
		};

	static const char lpErrMsgs[][64] = 
		{
			"Permission denied.", 
			"Address already in use.", 
			"Cannot assign requested address.", 
			"Address family not supported by protocol family.", 
			"Operation already in progress.", 
			"Software caused connection abort.", 
			"Connection refused.", 
			"Connection reset by peer.", 
			"Destination address required.", 
			"Bad address.", 
			"Host is down.", 
			"No route to host.", 
			"Operation now in progress.", 
			"Interrupted function call.", 
			"Invalid argument.", 
			"Socket is already connected.", 
			"Too many open sockets.", 
			"Message too long.", 
			"Network is down.", 
			"Network dropped connection on reset.", 
			"Network is unreachable.", 
			"No buffer space available.", 
			"Bad protocol option.", 
			"Socket is not connected.", 
			"Socket operation on nonsocket.", 
			"Operation not supported.", 
			"Protocol family not supported.", 
			"Too many processes.", 
			"Protocol not supported.", 
			"Protocol wrong type for socket.", 
			"Cannot send after socket shutdown.", 
			"Socket type not supported.", 
			"Connection timed out.", 
			"Resource temporarily unavailable.", 
			"Host not found.", 
			"Successful WSAStartup not yet performed.", 
			"Valid name, no data record of requested type.", 
			"This is a nonrecoverable error.", 
			"Network subsystem is unavailable.", 
			"Nonauthoritative host not found.", 
			"Winsock.dll version out of range.", 
			"Graceful shutdown in progress."
			"Mutex not created.", 
			"Thread not created."
		};

	for (int i = 0; i < sizeof (lErrCodes) / sizeof (long); i++)		
	{
		if (m_number == lErrCodes[i])
		{
			int slen = strlen (lpErrMsgs[i]);

			if (len > slen + 1) len = slen + 1;
			if (str) memcpy (str, lpErrMsgs[i], len);

			return slen;
		}
	}

	char lpUnknown[] = "Unknown error.";
	int slen = strlen (lpUnknown);

	if (len > slen + 1) len = slen + 1;
	if (str) memcpy (str, lpUnknown, len);
	return slen;
}

