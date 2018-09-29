#ifndef __9100NET_H__
#define	__9100NET_H__


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <pthread.h>

//#include "9100protocol.h"

#define	MAX_CLIENT		100
#define READ            0
#define WRITE           1
#define MTU             6500 //change 1500 2
#define SELECT

#define INFO_MAX_SZ 255
class C9100Net
{

public:
	C9100Net();
	~C9100Net();
	void ActiveCloseconnect();//�����ر�����

	void CheckOut(register char *cp,register int cnt );
	void wrtiebuffer(register char *cp,register int cnt );

	void UdpInit(int &LocalPort,char *pRemoteAddr,int &RemotePort);
	void TcpServerInit(int &LocalPort);
    void SendUdpData(char *parBuffer,int parlen);
    void RecvUdpData();

    void SendTcpData(int parsock, char *parBuffer,int parlen=MTU);
    static void SendTcpData(char *parBuffer,int parlen=MTU);//给自发收用
    //static void AnalyzeData(char *parBuffer,int parlen);

    pthread_t    threadRecv;
protected:
    static void* RecvThreadFun(void *lparam);
	static volatile bool bserverthreadrun;


	sockaddr_in RemoteAddr;
	sockaddr_in LocalAddr;
	int LocalLen;
public:
	int sock;//����socket
	//int snew;//accept ����


    static bool DealSelfRecAndSend(int parsoc);
    static int selfsock;//自发收的暂存socket;


    static bool DealStbRecAndSend(int parsoc);
};

#endif
