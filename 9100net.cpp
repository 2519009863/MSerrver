#include "9100net.h"
#include "9100protocol.h"
#include <errno.h>
#include <netdb.h>

#include <sys/epoll.h>
#include "CJsonObject.hpp"
using namespace std;
int C9100Net::selfsock=0;
C9100Pro *p9100Pro;
C9100Net *pNet;
volatile bool C9100Net::bserverthreadrun=true;
#define OPEN_MAX 100  //epoll监控句柄最大值
struct epoll_event event;   // 告诉内核要监听什么事件
struct epoll_event wait_event; //内核监听完的结果
struct connectedfd
{
    int fd;
    unsigned short peerport;
    unsigned int   peerip;
};
int maxi = 0;
connectedfd s_linkfd[OPEN_MAX];
int epfd;
//自发送
char *splitbuf;
char *recvbuffer;
char sendbuffer[MTU];
char headsendbuffer[MTU+1];

C9100Net::C9100Net()
{
	LocalLen=0;
	sock=0;


	p9100Pro=new C9100Pro();
	pNet=this;
}


C9100Net::~C9100Net()
{
    delete p9100Pro;p9100Pro=NULL;//xc mem
	close(sock);
	//close(snew);
}

void C9100Net::CheckOut(register char *cp,register int cnt )
{
	register char checksum;
	register int tmp;
	tmp = cnt;

	checksum = 0;
	while( tmp-- > 1)
	{
		checksum ^= *cp++;
	}
	*cp = checksum;
}
void C9100Net::wrtiebuffer(register char *cp,register int cnt )
{
	/*register char c;
	register int tmp;

	tmp = cnt;
	c = 0;

	while( tmp-- > 1)  {
		while( !isprint((c&0x7F)) )  c++;
		*cp++ = (c++&0x7F);
	}*/

}

void C9100Net::UdpInit(int &LocalPort,char *pRemoteAddr,int &RemotePort)
{
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port =htons(LocalPort);  //用于接收的端口号
	LocalAddr.sin_addr.s_addr =htonl( INADDR_ANY );
	LocalLen = sizeof( LocalAddr );

	sock = socket( AF_INET, SOCK_DGRAM, /*IPPROTO_RAW*/IPPROTO_UDP);
	//ace d
	/*int errno=bind( sock, (struct sockaddr *)&LocalAddr, sizeof( LocalAddr ) );
	if(errno<0)
	{

		printf("bind error ddd: %s\n", strerror(sock));
		close(sock);
		return;
	}*/


	memset( &RemoteAddr, 0, sizeof( RemoteAddr ) );
	int serverLen = sizeof( RemoteAddr );
	RemoteAddr.sin_family = AF_INET;
	RemoteAddr.sin_port = htons( RemotePort );
	RemoteAddr.sin_addr.s_addr = inet_addr(pRemoteAddr);

	pthread_create(&threadRecv, NULL,RecvThreadFun,(void *) &sock);
}

void C9100Net::SendUdpData(char *parBuffer,int parlen)
{

	sendto( sock, parBuffer,parlen*sizeof(char),0,(struct sockaddr *)&RemoteAddr,sizeof( RemoteAddr ));
}


void C9100Net::RecvUdpData()
{
	char buffer[MTU];
	int res=recvfrom( sock, buffer, MTU, 0,(struct sockaddr *)&LocalAddr, (socklen_t*)&LocalLen);
}


void C9100Net::SendTcpData(int parsock, char *parBuffer,int parlen)
{
    //printf("send tcp %s\n",parBuffer);

    send(parsock,( char*)parBuffer,parlen*sizeof(char),0);
    //printf("send over\n");
}

void C9100Net::SendTcpData( char *parBuffer,int parlen)
{
    send(selfsock,( char*)parBuffer,parlen*sizeof(char),0);
}
void C9100Net::TcpServerInit(int &LocalPort)
{

	int RemoteLen,LocalLen;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port =htons(LocalPort);
	LocalAddr.sin_addr.s_addr = htonl( INADDR_ANY );//s_addr= htonl( INADDR_ANY );//.S_un.S_addr = inet_addr("192.168.1.113");
    //LocalAddr.sin_addr.s_addr = inet_addr("192.168.1.102");
	LocalLen = sizeof( LocalAddr );

	sock = socket( AF_INET,  SOCK_STREAM, IPPROTO_TCP);
	if( sock <0 )
	{
		printf( "creat sock error!\n" );
		return;
	}
    int on = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        printf( "setsockopt error\n" );
        return;
    }

	if( bind( sock, ( struct sockaddr * )&LocalAddr, sizeof( LocalAddr) )<0)
	{
		printf( "bind error.\n" );
		close(sock);
		return;
	}
	if( listen( sock,  SOMAXCONN) <0 )
	{
		printf( "listen error\n" );
		close(sock);
		return;

	}


    int i = 0;//, maxi = 0;
    //int fd[OPEN_MAX];
    //memset(fd,-1, sizeof(fd));
    //fd[0] = sock;
    //connectedfd s_linkfd[OPEN_MAX];
    for(int m=0;m<OPEN_MAX;m++)//给fd初始化
    {
        s_linkfd[m].fd=-1;
    }
    s_linkfd[0].fd=sock;

    epfd = epoll_create(10); // 创建一个 epoll 的句柄，参数要大于 0， 没有太大意义
    if( -1 == epfd ){
        perror ("epoll_create");
        return ;
    }

    event.data.fd = sock;     //监听套接字
    event.events = EPOLLIN; // 表示对应的文件描述符可以读

    //5.事件注册函数，将监听套接字描述符 sockfd 加入监听事件
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event);
    if(-1 == ret){
        perror("epoll_ctl");
        return ;
    }

    printf("ready RecvThreadFun777\n");
    recvbuffer =( char * ) malloc(MTU); //未写入线程
    RecvThreadFun((void *) &sock);
    //pthread_create(&threadRecv, NULL,RecvThreadFun,(void *) &sock);
    //pthread_join(threadRecv, NULL);//�ȴ������߳̽���*/


    /*printf( "listening..........\n" );
    snew = accept( sock,NULL,NULL);//( struct sockaddr * )&RemoteAddr, &RemoteLen);

    //if(g_pTSParser)//��������ķ����߳������ ������Զ�̼��
    //  continue;

    if( snew <0 )
    {
        printf("accept error.res= %d\n",snew); //有时会出这个错儿
        printf("socket() failed: %s\n", strerror(errno));
        close(sock);
        return;
    }
    else
    {
        printf("ni da ye connected OK.\n");
        //p9100Pro->InitSnifferAndAnalyzer();
        //emit signetstat(0);
    }
    bserverthreadrun=true;
    RecvThreadFun(&snew);

    //pthread_create(&threadRecv, NULL,RecvThreadFun,(void *) &snew);
    //pthread_join(threadRecv, NULL);//�ȴ������߳̽���*/

}


int ttt=0;
bool bnetrun=true;
void* C9100Net::RecvThreadFun(void *lparam)
{
	int *s = (int*)lparam;

	//char *buffer=NULL;
	//buffer =( char * ) malloc(MTU);

    //char *splitbuf;
    //char sendbuffer[MTU]={0};
    //char headsendbuffer[MTU+1]={0};
    int ret;int i=0;
    while(1) //是否放到线程里
    {
        // 监视并等待多个文件（标准输入，udp套接字）描述符的属性变化（是否可读）
        // 没有属性变化，这个函数会阻塞，直到有变化才往下执行，这里没有设置超时
        ret = epoll_wait(epfd, &wait_event, maxi+1, -1);

        //printf("aafter epoll_wait %d\n",ret);
        //6.1监测sockfd(监听套接字)是否存在连接
        if(( *s == wait_event.data.fd )
           && ( EPOLLIN == wait_event.events & EPOLLIN ) )
        {
            struct sockaddr_in cli_addr;
            socklen_t clilen = sizeof(cli_addr);

            //6.1.1 从tcp完成连接中提取客户端
            //printf("before accept\n");
            int connfd = accept(*s, (struct sockaddr *)&cli_addr, &clilen);
            //int connfd = accept(sock, NULL, NULL);//
            //getpeername(connfd,(struct sockaddr *)&cli_addr, &clilen);

           // printf("peer port %d\n",htons(cli_addr.sin_port));
           // printf("after accept\n");
            //6.1.2 将提取到的connfd放入fd数组中，以便下面轮询客户端套接字
            for(i=1; i<OPEN_MAX; i++)
            {
                if(s_linkfd[i].fd< 0)//再有其他连接 依次往后排
                {
                    s_linkfd[i].fd = connfd;
                    s_linkfd[i].peerport=htons(cli_addr.sin_port);//目前根据对端的端口号来区分连接 还有其他办法 如私有协议
                    if(s_linkfd[i].peerport==43558)
                    {
                        selfsock=connfd;
                    }
                    event.data.fd = connfd; //监听套接字
                    event.events = EPOLLIN; // 表示对应的文件描述符可以读

                    //6.1.3.事件注册函数，将监听套接字描述符 connfd 加入监听事件
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                    if(-1 == ret){
                        perror("epoll_ctl   www");
                        return NULL;
                    }
                    printf("add connect\n");
                    break;
                }
            }

            //6.1.4 maxi更新
            if(i > maxi)
                maxi = i;

            //6.1.5 如果没有就绪的描述符，就继续epoll监测，否则继续向下看
            if(--ret <= 0)
                continue;
        }

        //6.2继续响应就绪的描述符
        for(i=1; i<=maxi; i++)//从已连接的socket中处理读写事件
        {
            if(s_linkfd[i].fd < 0)
                continue;

            if(( s_linkfd[i].fd == wait_event.data.fd )
               && ( EPOLLIN == wait_event.events & (EPOLLIN|EPOLLERR) ))
            {
                //////开始分类处理不同的链接
                //自连接根据peer端口判断 写死
                //if(s_linkfd[i].peerport==43558)
                {
              //      printf("before DealSelfRecAndSend\n");
                    //bool res=DealSelfRecAndSend(s_linkfd[i].fd);
                    bool res=DealStbRecAndSend(s_linkfd[i].fd);
                    if(res==false)
                    {
                        //说明已经关闭 从epoll中删除事件
                        epoll_ctl(epfd,EPOLL_CTL_DEL,s_linkfd[i].fd,NULL);
                        close(s_linkfd[i].fd);
                        s_linkfd[i].fd=-1; //保证再有连接 还往这个坑里填
                        printf("client close , normal close \n");
                    }

                }
                //else//对接机顶盒自测系统
                //{

                //}

                //6.2.2所有的就绪描述符处理完了，就退出当前的for循环，继续poll监测
                if(--ret <= 0)
                    break;
            }
        }
    }


    //delete buffer;buffer=NULL;//xc mem
}

bool C9100Net::DealStbRecAndSend(int parsoc)
{
    memset( recvbuffer, 0, MTU );
    memset( sendbuffer, 0, MTU );
    memset( headsendbuffer, 0, MTU+1);
    int len = read(parsoc,recvbuffer,MTU);
    if(len==0)
    {
        printf("len==0 \n"); //0代表服务器关闭socket android 7.1以上 不让连吗？
        perror("read");
        return false;//test
        //bnetrun=false;
        // break;
    }
    if(len>0)
    {
        //neb::CJsonObject oJson;
        string s_rec = recvbuffer;
        printf("recv  :%s\n",s_rec.data());
        neb::CJsonObject oJson(s_rec);
        //oJson.Add(s_rec);
        std::string strValue;
        oJson.Get("cmd", strValue);
        //printf("recv  %s\n",oJson.ToString().data());
        printf("cmd value %s\n",strValue.data());


        //解析协议 回传
        neb::CJsonObject sJson;
        if(strValue=="iptv:connect")
        {
            int res=p9100Pro->StartSniffer("eth0");
            if(res<0)
            {
                sJson.Add("cmd","iptv:connect");
                sJson.Add("statusCode",1);
                sJson.Add("errorMsg","权限异常");
                neb::CJsonObject sJsonResult("");//没实现显示大括号 可能会有问题 调试时再说
                sJson.Add("result",sJsonResult);
                printf("get sock error\n");
            }
            else
            {
                sJson.Add("cmd","iptv:connect");
                sJson.Add("statusCode",0);
                sJson.Add("errorMsg","");
                neb::CJsonObject sJsonResult("");//没实现显示大括号 可能会有问题 调试时再说

                neb::CJsonObject sJsonResultsub("");
                sJsonResult.Add("",sJsonResultsub);

                sJson.Add("result",sJsonResult);
                printf("get sock ok----\n");
            }

        }

        else if(strValue=="iptv:get_channel_num")
        {
            int num=0;
            if(p9100Pro->GetIpNum(num))
            {
                sJson.Add("cmd","iptv:get_channel_num");
                sJson.Add("statusCode",0);
                neb::CJsonObject sJsonResult("");//没实现显示大括号 可能会有问题 调试时再说
                sJsonResult.Add("numList",num);
                sJson.Add("result",sJsonResult);

            } else
            {
                sJson.Add("cmd","iptv:get_channel_num");
                sJson.Add("statusCode",1);
            }
        }
        else if(strValue=="iptv:get_channel_list")
        {
            int num=0;
            p9100Pro->GetIpNum(num);
            if(num>0)
            {
                p9100Pro->GetIpListForStb();
                //string striplist=sendbuffer;
                //char s[] = "a,b*c,d";

                //test
                //char ssbuf[200]={0};
                //strcpy(ssbuf,"123:56.78.9.0.6:454546:er:ererrtr");
                /*const char *sep = ":"; //可按多个字符来分割
                char Element[50][100];//目前最大支持10个列表
                int iindex=0;
                Element[iindex] = strtok(sendbuffer, sep);
                while(Element[iindex]){
                    printf("%s\n", Element[iindex]);
                    iindex++;
                    Element[iindex] = strtok(NULL, sep);
                }
                printf("\n");*/


                sJson.Add("cmd","get_channel_list");
                sJson.Add("statusCode",0);

                //数组例子
                sJson.AddEmptySubArray("result");
                neb::CJsonObject sJsonResult[MAX_IPTVLIST];
                for(int i=0;i<num;i++)
                {
                    sJsonResult[i].Add("sourceIp",p9100Pro->StbIptvList[i].SrcIP);
                    sJsonResult[i].Add("sourcePort",p9100Pro->StbIptvList[i].srcPort);
                    sJsonResult[i].Add("destIp",p9100Pro->StbIptvList[i].DesIP);
                    sJsonResult[i].Add("destPort",p9100Pro->StbIptvList[i].desPort);
                    sJsonResult[i].Add("proto",p9100Pro->StbIptvList[i].Protol);
                    sJsonResult[i].Add("rate",p9100Pro->StbIptvList[i].speed);
                    sJson["result"].Add(sJsonResult[i]);
                }

            }
            else
            {
              printf("ipnum==0\n");
            }

        }
        else if(strValue=="iptv:start_monitor")//这条指令先不作操作
        {
            //默认分析第一条IPTV
            char asrcport[10];
            sprintf(asrcport,"%d",p9100Pro->StbIptvList[0].srcPort);

            char adesport[10];
            sprintf(adesport,"%d",p9100Pro->StbIptvList[0].desPort);

            p9100Pro->SetCurIptvList(p9100Pro->StbIptvList[0].SrcIP,asrcport
                    ,p9100Pro->StbIptvList[0].DesIP,adesport,p9100Pro->StbIptvList[0].Protol);//计划后期去掉
            p9100Pro->StartAnalyzer();

            sJson.Add("cmd","iptv:start_monitor");
            sJson.Add("statusCode",0);
            printf("get start_monitor ok\n");
        }
        else if(strValue=="iptv:get_monitor_data")
        {
            p9100Pro->GetMonitorDataForStb();
            sJson.Add("cmd","iptv:get_monitor_data");
            sJson.Add("statusCode",0);
            sJson.AddEmptySubArray("result");
            neb::CJsonObject sJsonResult[MAX_IPTVLIST];
            for(int i=0;i<1;i++)
            {
                sJsonResult[i].Add("ipRate",p9100Pro->StbMonitorData[i].ipRate);
                sJsonResult[i].Add("delayFactor",p9100Pro->StbMonitorData[i].delayFactor);
                sJsonResult[i].Add("packetLostRate",p9100Pro->StbMonitorData[i].packetLostRate);
                sJsonResult[i].Add("packetLostCount",p9100Pro->StbMonitorData[i].packetLostCount);
                sJsonResult[i].Add("videoMos",p9100Pro->StbMonitorData[i].videoMos);
                sJsonResult[i].Add("audioMos",p9100Pro->StbMonitorData[i].audioMos);
                sJsonResult[i].Add("igmpSwitchTime",p9100Pro->StbMonitorData[i].igmpSwitchTime);
                sJsonResult[i].Add("videoRate",p9100Pro->StbMonitorData[i].videoRate);
                sJsonResult[i].Add("audioRate",p9100Pro->StbMonitorData[i].audioRate);
                sJsonResult[i].Add("allPacketNumber",p9100Pro->StbMonitorData[i].allPacketNumber);
                sJsonResult[i].Add("wholeLossNumber",p9100Pro->StbMonitorData[i].wholeLossNumber);
                sJsonResult[i].Add("outSequencePacketNumber",p9100Pro->StbMonitorData[i].outSequencePacketNumber);
                sJsonResult[i].Add("duplicationPacketNumber",p9100Pro->StbMonitorData[i].duplicationPacketNumber);
                sJson["result"].Add(sJsonResult[i]);
            }
            printf("----------------------------------------------------------\n");

        }
        else if(strValue=="iptv:stop_monitor")
        {
            p9100Pro->StopAnalyzer();
            sJson.Add("cmd","iptv:stop_monitor");
            sJson.Add("statusCode",0);
        }
        else if(strValue=="iptv:change_channel_start")
        {
            p9100Pro->StartIGMP();
            sJson.Add("cmd","iptv:change_channel_start");
            sJson.Add("statusCode",0);
        }

        else if(strValue=="iptv:change_channel_press")
        {
            long fleave=1;
            long fjoin=1;
            printf("1\n");
            p9100Pro->GetChannelChange(fleave,fjoin);printf("2\n");
            sJson.Add("cmd","iptv:change_channel_press");
            sJson.Add("statusCode",0);
            sJson.Add("leave",(float)fleave);printf("3\n");
            sJson.Add("join",(float)fjoin);
        }
        else if(strValue=="iptv:change_channel_stop")
        {
            p9100Pro->StopIGMP();
            sJson.Add("cmd","iptv:change_channel_stop");
            sJson.Add("statusCode",0);
        }
        string strSend=sJson.ToString();//sJson.ToFormattedString();
        strSend=strSend+"\r\n";
        printf("send  :%s\n",strSend.data());
        pNet->SendTcpData(parsoc,(char*)strSend.data(),strSend.length());

        /////send  {"cmd":["iptv:start_monitor"],"statusCode":0}

        /*sJson.Add("cmd","iptv:connect");
        sJson.Add("statusCode",0);
        sJson.Add("errorMsg","");
        neb::CJsonObject sJsonResult("");//没实现显示大括号 可能会有问题 调试时再说
        //sJsonResult.Add("numList",5);
        //sJsonResult.Add("","");
        sJson.Add("result",sJsonResult);

        //数组例子
        sJson.AddEmptySubArray("result");
        neb::CJsonObject sJsonResult1("");
        sJsonResult1.Add("ipRate",1.1);
        sJsonResult1.Add("sourceIp","x.x.x.x");
        sJson["result"].Add(sJsonResult1);
        neb::CJsonObject sJsonResult2("");
        sJsonResult2.Add("ipRate",2.1);
        sJsonResult2.Add("sourceIp","e.e.e.e");
        sJson["result"].Add(sJsonResult2);*/




        return true;
    }
}

bool C9100Net::DealSelfRecAndSend(int parsoc)
{
    memset( recvbuffer, 0, MTU );
    memset( sendbuffer, 0, MTU );
    memset( headsendbuffer, 0, MTU+1);
    int len = read(parsoc,recvbuffer,MTU);
    if(len==0)
    {
        printf("len==0 \n"); //0代表服务器关闭socket android 7.1以上 不让连吗？
        perror("read");
        //bnetrun=false;
        // break;
        return false;
    }
    if(len>0)
    {
        printf("len==%d \n",len); //0代表服务器关闭socket android 7.1以上 不让连吗？

        char *sectpro[30];
        char *outer_ptr=NULL;
        char *inner_ptr=NULL;
        int in=0;
        int  sendlen=0;

        splitbuf=recvbuffer;
        //printf("after splitbuf\n");
        while((sectpro[in]=strtok_r(splitbuf,":",&outer_ptr))!=NULL&&in<20) {
            splitbuf=sectpro[in];
            in++;//add
            splitbuf=NULL;
        }
        //printf("after sectpro\n");
        if(!strcmp(sectpro[0],"IPTV"))
        {
            if(!strcmp(sectpro[1],"Control"))
            {
                if(!strcmp(sectpro[2],"StartCapture"))
                {
                    int res=p9100Pro->StartSniffer(sectpro[3]);
                    if(res<0)
                    {
                        sprintf(sendbuffer,"Aget sock error\r\n");
                        printf("get sock error\n");
                    }
                    else
                    {
                        sprintf(sendbuffer,"Aget sock ok\r\n");
                        printf("get sock ok\n");
                    }
                    pNet->SendTcpData(parsoc,sendbuffer,15);
                }
                else if(!strcmp(sectpro[2],"StartJSHTCapture"))
                {
                    int res=p9100Pro->StartJSHTSniffer(sectpro[3]);
                    if(res<0)
                    {
                        sprintf(sendbuffer,"Aget sock error\r\n");
                    }
                    else
                    {
                        sprintf(sendbuffer,"Aget sock ok\r\n");
                    }
                    pNet->SendTcpData(parsoc,sendbuffer,15);
                }
                else if(!strcmp(sectpro[2],"StartTcpAnalyzer"))
                {
                    ;//p9100Pro->StartTcpAnalyzer();
                }
                else if(!strcmp(sectpro[2],"StopAnalyzer"))
                {
                    p9100Pro->StopAnalyzer();
                }
                else if(!strcmp(sectpro[2],"DeleteServSock"))
                {
                    //printf("before DelSnifferAndAnalyzer");
                    //p9100Pro->DelSnifferAndAnalyzer();
                    ;// bnetrun=false;

                }
                else if(!strcmp(sectpro[2],"BeginSendForDec"))
                {
                    p9100Pro->StartDec();
                }
                else if(!strcmp(sectpro[2],"StopSendForDec"))
                {
                    p9100Pro->StopDec();
                }
                else if(!strcmp(sectpro[2],"StopSavePcap"))
                {
                    p9100Pro->StopSavePcap();
                }
                else if(!strcmp(sectpro[2],"StartIGMP"))
                {
                    p9100Pro->StartIGMP();
                }
                else if(!strcmp(sectpro[2],"StopIGMP"))
                {
                    p9100Pro->StopIGMP();
                }
                else if(!strcmp(sectpro[2],"StartHLS"))
                {
                    p9100Pro->StartHLS();
                }
                else if(!strcmp(sectpro[2],"StopHLS"))
                {
                    p9100Pro->StopHLS();
                }
                else if(!strcmp(sectpro[2],"StartHTTP"))
                {
                    p9100Pro->StartHTTP();
                }
                else if(!strcmp(sectpro[2],"StopHTTP"))
                {
                    p9100Pro->StopHTTP();
                }
                else if(!strcmp(sectpro[2],"StartHLSHea"))
                {
                    p9100Pro->StartHLSHealth();
                }
                else if(!strcmp(sectpro[2],"StopHLSHea"))
                {
                    p9100Pro->StopHLSHealth();
                }
            }
            else if(!strcmp(sectpro[1],"Set"))
            {
                if(!strcmp(sectpro[2],"CurIpList"))
                {
                    p9100Pro->SetCurIptvList(sectpro[3],sectpro[4],sectpro[5],sectpro[6],sectpro[7]);
                    p9100Pro->StartAnalyzer();
                }
                if(!strcmp(sectpro[2],"JSHTAnalyzer"))
                {
                    //p9100Pro->SetCurIptvList(sectpro[3],sectpro[4],sectpro[5],sectpro[6]);
                    bool budp=false;bool bjsht=false;
                    if(atoi(sectpro[5])==1)
                        budp=true;
                    else
                        budp=false;

                    if(atoi(sectpro[6])==1)
                        bjsht=true;
                    else
                        bjsht=false;


                    bool res=p9100Pro->BeginJSHTAnalyzer(atoi(sectpro[3]),sectpro[4],budp,bjsht);
                    if(res<0)
                    {
                        sprintf(sendbuffer,"O0");
                    }
                    else
                    {
                        sprintf(sendbuffer,"O1");
                    }
                    pNet->SendTcpData(parsoc,sendbuffer,5);

                }

                if(!strcmp(sectpro[2],"IfFilter"))
                {
                    p9100Pro->SetIfFilter(sectpro[3],sectpro[4],sectpro[5]);
                }
                if(!strcmp(sectpro[2],"TSName"))
                {
                    p9100Pro->SetTSName(sectpro[3]);
                }
                if(!strcmp(sectpro[2],"PCAPName"))
                {
                    p9100Pro->SetPcapNameAndBegin(sectpro[3]);
                }
                if(!strcmp(sectpro[2],"TSRatioTimePer"))
                {
                    p9100Pro->TSRatioTimePer(sectpro[3]);
                }
            }
            else if(!strcmp(sectpro[1],"Data"))
            {
                if(!strcmp(sectpro[2],"GetTcpFillFinish"))
                {
                    int res=0;
                    p9100Pro->GetTcpFill(res);
                    sprintf(sendbuffer,"E%d\r\n",res);
                    pNet->SendTcpData(parsoc,sendbuffer,10);

                }
                if(!strcmp(sectpro[2],"GetIptvNum"))
                { //ttt++;printf("GetIptvNum---%d\n",ttt);
                    int num=0;
                    if(p9100Pro->GetIpNum(num))
                    {
                        sprintf(sendbuffer,"B%d\r\n",num);
                        pNet->SendTcpData(parsoc,sendbuffer,10);

                    }

                }
                if(!strcmp(sectpro[2],"GetIptvList"))
                {
                    p9100Pro->GetIpList(sendbuffer);
                    //memset(sendbuffer,5,MTU*5);
                    headsendbuffer[0]='C';
                    memcpy(headsendbuffer+1,sendbuffer,strlen(sendbuffer)+1);
                    pNet->SendTcpData(parsoc,headsendbuffer);

                }
                if(!strcmp(sectpro[2],"GetMosV"))
                {
                    if(p9100Pro->GetMosV(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetMosDetail"))
                {
                    if(p9100Pro->GetMosDetail(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetSavedMB"))
                {
                    if(p9100Pro->GetSavedMB(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"Get4445Wave"))
                {

                    if(p9100Pro->Get4445(sendbuffer,sendlen))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,sendlen);
                    }
                }
                if(!strcmp(sectpro[2],"Get4445Detail"))
                {

                    if(p9100Pro->Get4445Detail(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetTr290"))
                {

                    if(p9100Pro->GetTr290(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetPCR"))
                {

                    if(p9100Pro->GetPCR(sendbuffer,sendlen))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,sendlen);
                    }
                }
                if(!strcmp(sectpro[2],"GetPSI"))
                {

                    if(p9100Pro->GetPSI(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);
                    }
                    else
                    {
                        printf("888\n");

                    }
                }
                if(!strcmp(sectpro[2],"GetMIT"))
                {

                    if(p9100Pro->GetIPONMIT(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);
                    }
                    else
                    {
                        printf("999\n");

                    }
                }
                if(!strcmp(sectpro[2],"GetMainFactor"))
                {
                    if(p9100Pro->GetMainFactor(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetVideoBuffer"))
                {
                    if(p9100Pro->GetVideoBuffer(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetTsRatioTime"))
                {
                    if(p9100Pro->GetTsratioTime(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"GetESMain"))
                {
                    if(p9100Pro->GetEsMain(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
                if(!strcmp(sectpro[2],"Get126"))
                {
                    if(p9100Pro->Get126(sendbuffer))
                    {
                        pNet->SendTcpData(parsoc,sendbuffer,strlen(sendbuffer)+1);

                    }
                }
            }
            else
            {
                ;
            }

        }
       return true;
    }
}