#include "9100protocol.h"
#include <string>
using namespace std;
#define DRAW_PCR_NUM 400
#define DRAW_MDI_NUM 50

CTSParser *g_pSerTSParser=NULL;
struIpList listtmp;
extern bool bAddFilter;
extern short sIpType;
extern struTr101290Param mstruTr101290Param;
extern bool banalyzeigmp;
extern bool banalyzehttp;
extern bool banalyzem3u8;
extern bool banalyzehlshealth;
pthread_t       threadPacket_t;
void* ThreadPacketFun(void *lparam);
void* ThreadPacketFun(void *lparam)
{
    g_pSerTSParser->m_bThreadRun=true;
    g_pSerTSParser->IPMultInserting();

}
C9100Pro::C9100Pro(void)
{

}
C9100Pro::~C9100Pro(void)
{
	DelSnifferAndAnalyzer();//xc mem
}

void C9100Pro::InitSnifferAndAnalyzer()
{

    if(!g_pSerTSParser)
    	{
		g_pSerTSParser=new CTSParser;
		printf("init over\n");
		g_pSerTSParser->InitParseParam(true, true);//after init insert data
	}
}

void C9100Pro::DelSnifferAndAnalyzer()//暂时先不调用 。 初始化一次 不删除C9100Pro
{
    g_pSerTSParser->m_bThreadRun=false;//停止接收线程
    usleep(1000);
    //关闭裸体套接字
    printf("before close s\n");
    close(g_pSerTSParser->s);


	if(g_pSerTSParser)
		{
		g_pSerTSParser->StopReceiveAndParse();
		
		delete g_pSerTSParser;
		g_pSerTSParser=NULL;
		printf("delete g_pSerTSParser\n");
	}

}
int C9100Pro::StartSniffer(char *c_in)
{
    int res=-2;
    if(g_pSerTSParser==NULL)
    {
        g_pSerTSParser=new CTSParser;
        g_pSerTSParser->InitParseParam(true, true);
		g_pSerTSParser->mfJSHT=false;
        res=g_pSerTSParser->OpenEthName(c_in);
        printf("after OpenEthName\n");
        pthread_create(&threadPacket_t, NULL,ThreadPacketFun,(void *) 0);
    }
    return res;
}
int C9100Pro::StartJSHTSniffer(char *c_in)
{
	int res=-2;
	if(g_pSerTSParser==NULL)
	{
		g_pSerTSParser=new CTSParser;
		g_pSerTSParser->InitParseParam(true, true);
		g_pSerTSParser->mfJSHT=true;
		res=g_pSerTSParser->OpenEthName(c_in);
		pthread_create(&threadPacket_t, NULL,ThreadPacketFun,(void *) 0);
	}
	return res;
}
bool C9100Pro::BeginJSHTAnalyzer(int desport,char * desIP,bool budp,bool bjsht)
{
	return g_pSerTSParser->BeginAnalyzer(desport,desIP,budp,bjsht);
}
void C9100Pro::StartAnalyzer()
{
    g_pSerTSParser->bdecideIPTV=false; //stop insert data then StopReceiveAndParse
    usleep(30000);
    g_pSerTSParser->StopReceiveAndParse();
    g_pSerTSParser->InitParseParam(true, true);//after init insert data

    g_pSerTSParser->bdecideIPTV=true;
    g_pSerTSParser->SetIpPara(/*Iplist[row]*/listtmp);
    if(listtmp.IptvType==TcpIptv)
    {

      //  return;//不分开 操作 在这配置延时
        while(!g_pSerTSParser->bTcpFill)
        {
            usleep(200000);
            printf("waiting-------\n");
        }
    }
    g_pSerTSParser->StartReceive(NULL,true,true);
    g_pSerTSParser->StartParse(true);
    g_pSerTSParser->SetVideoMosThreshold(0,3);
    g_pSerTSParser->SetAudioMosThreshold(0,3);
    g_pSerTSParser->SetPcrAndMdiQuant(400,50);//DRAW_PCR_NUM 400#define DRAW_MDI_NUM 50

}
void C9100Pro::StartTcpAnalyzer()
{
    g_pSerTSParser->StartReceive(NULL,true,true);
    g_pSerTSParser->StartParse(true);
    g_pSerTSParser->SetVideoMosThreshold(0,3);
    g_pSerTSParser->SetAudioMosThreshold(0,3);
    g_pSerTSParser->SetPcrAndMdiQuant(400,50);//DRAW_PCR_NUM 400#define DRAW_MDI_NUM 50
}
void C9100Pro::StopAnalyzer()
{
    if(g_pSerTSParser->ParseThreadBegined)
    {
        printf("rttttttttttttttttt  stop\n");
        g_pSerTSParser->bdecideIPTV=false;
        g_pSerTSParser->StopReceiveAndParse();
        g_pSerTSParser->InitParseParam(true, true);
        g_pSerTSParser->bdecideIPTV=true;
    }
}

void C9100Pro::SetIfFilter(char *ptotal,char *pDesIp,char *pDesPort)
{

}
void C9100Pro::SetPcapNameAndBegin(char *name)
{
    char pcapnamebuf[100]={0};
    strcpy(pcapnamebuf,name);

    char cpath[20]="/sdcard/";
    strcat(cpath,pcapnamebuf);
    strcat(cpath,".pcap");
    g_pSerTSParser->writefile(cpath);//when FileDialog open  not sniffer no write data

}

void C9100Pro::TSRatioTimePer(char *name)
{
    g_pSerTSParser->SetTSRatioTimePer(name);

}

void C9100Pro::StopSavePcap()
{
    g_pSerTSParser->stopwritefile();
    system("sync");
}
void C9100Pro::SetTSName(char *name)
{
	g_pSerTSParser->SetTSname(name);
}
void C9100Pro::GetTcpFill(int &ires)
{
    if(g_pSerTSParser->bTcpFill)
        ires=1;
    else
        ires=0;
}
bool C9100Pro::GetIpNum(int &IpNum)
{
	IpNum  =g_pSerTSParser->GetIpNum();
    if(IpNum==0)
    	return false;
    else
    	return true;
}
void C9100Pro::StartDec()
{
    g_pSerTSParser->bsendfordec=true; //stop fill analyzer list
    if(g_pSerTSParser->bSaveTs)
    {
    	 g_pSerTSParser->StartSaveTsFile();//xcsave
    }
    else
         g_pSerTSParser->BeginHuanchong();
}
void C9100Pro::StopDec()
{
	if(g_pSerTSParser->bsendfordec==true)
	{
		g_pSerTSParser->bsendfordec=false;
		if(g_pSerTSParser->bSaveTs)
		{
			g_pSerTSParser->StopSaveTsFile();//xcsave
		}
		else
			g_pSerTSParser->StopHuanchong();
	}

}
void C9100Pro::StartIGMP()
{
	banalyzeigmp=true;
}
void C9100Pro::StopIGMP()
{
	banalyzeigmp=false;
}

void C9100Pro::StartHLSHealth()
{
    printf("start hh\n");
    banalyzehttp=true;
    banalyzem3u8=true;
    banalyzehlshealth=true;
}
void C9100Pro::StopHLSHealth()
{
	banalyzehlshealth=false;
}

void C9100Pro::StartHLS()
{
    printf("start HLSS\n");
    banalyzehttp=true;
    banalyzem3u8=true;
}
void C9100Pro::StopHLS()
{
    banalyzehttp=false;
    banalyzem3u8=false;
}

void C9100Pro::StartHTTP()
{
    banalyzehttp=true;
}
void C9100Pro::StopHTTP()
{
    banalyzehttp=false;
}

bool C9100Pro::GetIpList(char *buf)
{
	struIpList *Iplist=g_pSerTSParser->GetIpList();
	int n=g_pSerTSParser->GetIpNum();
    if(n>0)
    {
       unsigned int isrc=1111;
       unsigned int ides=2222;
       char csrc[50];
       char cdes[50];
       char smallbuf[n][200];
       for(int i=0;i<n;i++)
       {
    	   isrc=htonl(Iplist[i].SourceIp);
    	   ides=htonl(Iplist[i].DestIp);

    	   //csrc=inet_ntoa((in_addr&)isrc);
    	   //cdes=inet_ntoa((in_addr&)ides);
    	   sprintf (csrc, "%u.%u.%u.%u",
    	   (unsigned char) * ((char *) &isrc + 0),
    	   (unsigned char) * ((char *) &isrc + 1),
    	   (unsigned char) * ((char *) &isrc + 2), (unsigned char) * ((char *) &isrc + 3));

    	   sprintf (cdes, "%u.%u.%u.%u",
    	   (unsigned char) * ((char *) &ides + 0),
    	   (unsigned char) * ((char *) &ides + 1),
    	   (unsigned char) * ((char *) &ides + 2), (unsigned char) * ((char *) &ides + 3));

    	   //printf("src %s\n",csrc);
    	   //printf("des %s\n",cdes);
    	   if(Iplist[i].IptvType==UdpIptv)
    	     sprintf(smallbuf[i],"%s:%d:%s:%d:udp:%5.2f:%d:",csrc,Iplist[i].SourcePort
    		   ,cdes,Iplist[i].DestPort,Iplist[i].PacketRate,Iplist[i].Ipttl);
    	   else if(Iplist[i].IptvType==RtpIptv)
    	     sprintf(smallbuf[i],"%s:%d:%s:%d:rtp:%5.2f:%d:",csrc,Iplist[i].SourcePort
    		   ,cdes,Iplist[i].DestPort,Iplist[i].PacketRate,Iplist[i].Ipttl);
    	   else
    		 sprintf(smallbuf[i],"%s:%d:%s:%d:tcp:%5.2f:%d:",csrc,Iplist[i].SourcePort
    		   ,cdes,Iplist[i].DestPort,Iplist[i].PacketRate,Iplist[i].Ipttl);

    	   strcat(buf,smallbuf[i]);
       }
       //strcat(buf,"\r\n");
    }
    return true;
}
bool C9100Pro::GetIpListForStb()
{
    struIpList *Iplist=g_pSerTSParser->GetIpList();
    int n=g_pSerTSParser->GetIpNum();
    if(n>0)
    {
        unsigned int isrc=1111;
        unsigned int ides=2222;
        char csrc[50];
        char cdes[50];
        //char smallbuf[n][200];
        char csrcport[15];
        char cdesport[15];
        char cspeed[10];
        for(int i=0;i<n;i++)
        {
            isrc=htonl(Iplist[i].SourceIp);
            ides=htonl(Iplist[i].DestIp);

            //csrc=inet_ntoa((in_addr&)isrc);
            //cdes=inet_ntoa((in_addr&)ides);
            sprintf (csrc, "%u.%u.%u.%u",
                     (unsigned char) * ((char *) &isrc + 0),
                     (unsigned char) * ((char *) &isrc + 1),
                     (unsigned char) * ((char *) &isrc + 2), (unsigned char) * ((char *) &isrc + 3));

            sprintf (cdes, "%u.%u.%u.%u",
                     (unsigned char) * ((char *) &ides + 0),
                     (unsigned char) * ((char *) &ides + 1),
                     (unsigned char) * ((char *) &ides + 2), (unsigned char) * ((char *) &ides + 3));


            strcpy(StbIptvList[i].SrcIP,csrc);
            StbIptvList[i].srcPort=Iplist[i].SourcePort;
            strcpy(StbIptvList[i].DesIP,cdes);
            StbIptvList[i].desPort=Iplist[i].DestPort;
            StbIptvList[i].speed=Iplist[i].PacketRate;

            if(Iplist[i].IptvType==UdpIptv)
            {
                strcpy(StbIptvList[i].Protol,"Udp");

            }
            else if(Iplist[i].IptvType==RtpIptv)
            {
                strcpy(StbIptvList[i].Protol,"Rtp");
            }
            else
            {
                strcpy(StbIptvList[i].Protol,"Tcp");
            }

        }
        //strcat(buf,"\r\n");
    }
    return true;
}

bool C9100Pro::GetMonitorDataForStb()
{
    double ipRate=g_pSerTSParser->GetIpRate();
    StbMonitorData[0].ipRate=ipRate;
    double idealdf=g_pSerTSParser->GetIpMdiDf_Expect();//maybe not
    StbMonitorData[0].delayFactor=idealdf;
    struIptvLoss sI=g_pSerTSParser->GetIptvParam();
    StbMonitorData[0].packetLostRate=sI.CurLostRate;
    u32     *pmdimlr=g_pSerTSParser->GetIpMLR();
    StbMonitorData[0].packetLostCount= *pmdimlr;
    struMos Mosv=g_pSerTSParser->GetMosVideo(0);
    StbMonitorData[0].igmpSwitchTime=500;
    StbMonitorData[0].videoMos=Mosv.MosCur;
    struMos Mosa=g_pSerTSParser->GetMosAudio(0);
    StbMonitorData[0].audioMos=Mosa.MosCur;
    struRatioInfo sr=g_pSerTSParser->GetRatioInfo();
    StbMonitorData[0].allPacketNumber=sI.AllPac;
    StbMonitorData[0].wholeLossNumber=sI.WholeLoss;
    StbMonitorData[0].outSequencePacketNumber=sI.OdQt;
    StbMonitorData[0].duplicationPacketNumber=sI.DplQt;

    return true;
}
bool C9100Pro::GetChannelChange(long fleave,long fjoin)
{
    long lus1=g_pSerTSParser->tjoin.tv_sec*1000000+g_pSerTSParser->tjoin.tv_usec;
    lus1=lus1/1000;
    fjoin=lus1;

    long lus2=g_pSerTSParser->tIgmpleave.tv_sec*1000000+g_pSerTSParser->tIgmpleave.tv_usec;
    lus2=lus2/1000;
    fleave=lus2;

    return true;

}
bool C9100Pro::GetMosV(char *buf)
{
	char cStreamtype[10];
	char cFramerate[10];
	char cAudiotype[10];

	struMos Mosv=g_pSerTSParser->GetMosVideo(0);

	double ipRate=g_pSerTSParser->GetIpRate();
    double *pmdiDF=g_pSerTSParser->GetIpMdiDf();
	u32     *pmdimlr=g_pSerTSParser->GetIpMLR();
	double *pipJitter=g_pSerTSParser->GetRtpJitter();

	struVideo sVideo=g_pSerTSParser->GetVideo(0);
	struAllGop *psAllGop=g_pSerTSParser->GetGopInfo(0);
	struPcr *sPcr=g_pSerTSParser->GetPcr(0);
    StruTr101290  Tr101290;
    Tr101290=g_pSerTSParser->GetTr101290();

    struPat m_patstruct=g_pSerTSParser->GetPat();

	if(!psAllGop) return false;
    if(!pipJitter) return false;
    if(!sPcr) return false;
    if(!pmdiDF) return false;
    if(!pmdimlr) return false;
	switch(sVideo.StreamType)
	{
	case 27:
        strcpy(cStreamtype,"H.264");
	break;
	default:
		strcpy(cStreamtype,"MPEG-2");
	break;
	}

	switch(sVideo.frame_rate_code)
	{
	case 3:
        strcpy(cFramerate,"25fbps");
	break;
	default:
		strcpy(cFramerate,"30fbps");
	break;
	}

	float f_epsnrtran=g_pSerTSParser->GetEpsnrTrans(0);
	float f_epsnr=g_pSerTSParser->GetEpsnrEnc(0);
	float f_epsnrAtis=g_pSerTSParser->GetEpsnrAtis(0);
	struProgBand sBand=g_pSerTSParser->GetProgBand(0);
    struMos sAmos=g_pSerTSParser->GetMosAudio(0);
	struRatioInfo sr=g_pSerTSParser->GetRatioInfo();
	StruTsRate sTT=g_pSerTSParser->GetTSRate();
	struIptvLoss sI=g_pSerTSParser->GetIptvParam();
	struAudio sA=g_pSerTSParser->GetAudio(0);
    int  rtppayloadtype =g_pSerTSParser->Rtp.Head.PayloadType;
	switch(sA.AudioInfo[0].StreamType)
  	{
     case 3:
	 case 4:
		strcpy(cAudiotype,"AAC");
		break;
	 case 0xf:
	 	strcpy(cAudiotype,"AAC");
		break;	 	
     case 0x81:
	 case 0x6:
	 	strcpy(cAudiotype,"AC3");
		break;  

     }
	

	char ipbuf[200]={0};
	char tsbuf[200]={0};
	char esbuf[200]={0};
	sprintf(ipbuf,"D%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,",Mosv.MosCur,Mosv.MosMax,Mosv.MosMin,f_epsnrtran,*pmdiDF);
	sprintf(tsbuf,"%d,%d,%d,%d,%d,%d,",*pmdimlr,sI.OdQt,sI.DplQt,Tr101290.ContinuityCountError,Tr101290.TsSyncLoss,Tr101290.PatError);
	sprintf(esbuf,"%d,%d,%G,%G,%s,%d,%d,%s,%d,%5.2f,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",
			m_patstruct.table_id,0,sPcr->CurrentAc,sPcr->CurrentInterval,cStreamtype,
			sVideo.horizontal_size,sVideo.vertical_size,cFramerate,rtppayloadtype,sTT.CurrentRate/1024
	         ,g_pSerTSParser->desMac[0],g_pSerTSParser->desMac[1],g_pSerTSParser->desMac[2],g_pSerTSParser->desMac[3],g_pSerTSParser->desMac[4],g_pSerTSParser->desMac[5]
	         ,g_pSerTSParser->srcMac[0],g_pSerTSParser->srcMac[1],g_pSerTSParser->srcMac[2],g_pSerTSParser->srcMac[3],g_pSerTSParser->srcMac[4],g_pSerTSParser->srcMac[5]);
    /*sprintf(esbuf,"%d",m_patstruct.table_id);printf("888\n");
    sprintf(esbuf,"%5.2f",sPcr->CurrentAc);printf("999\n");
    sprintf(esbuf,"%5.2f",sPcr->CurrentInterval);printf("zzz\n");
    sprintf(esbuf,"%s",cStreamtype);printf("xxx\n");
    sprintf(esbuf,"%d",sVideo.horizontal_size);printf("ccc\n");
    sprintf(esbuf,"%d",sVideo.vertical_size);printf("vvv\n");
    sprintf(esbuf,"%s",cFramerate);printf("bbb\n");*/

	strcat(buf,ipbuf);
	strcat(buf,tsbuf);
	strcat(buf,esbuf);
	/*sprintf(buf,"%5.2f,%5.2f,%5.2f,%d,%5.2f,%s,%d,%d,%d,%s,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%d/%d,%d,%d"
		    ,Mosv.MosCur,ipRate,*pmdiDF,*pmdimlr,*pipJitter,cStreamtype,psAllGop->CurGop.FrameQuant,sVideo.horizontal_size,sVideo.vertical_size,cFramerate
			,Mosv.MosMax,Mosv.MosMin,Mosv.MosAvr,Mosv.BelowRate*100,f_epsnrtran,f_epsnr,f_epsnrAtis,sBand.VideoBand[0]/1024/1024
			,sAmos.MosCur,sAmos.MosMax,sAmos.MosMin,sAmos.MosAvr,sAmos.BelowRate*100,sTT.CurrentRate*sr.AudioRatio/1024
			,sI.WholeLoss,sI.AllPac,sI.OdQt,sI.DplQt);*/
	return true;
}

bool C9100Pro::GetTr290(char *buf)
{
	StruTr101290  Tr101290;
	Tr101290=g_pSerTSParser->GetTr101290();
	
    sprintf(buf,"F%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",Tr101290.TsSyncLoss,Tr101290.SyncByteError,Tr101290.PatError
		             ,Tr101290.ContinuityCountError,Tr101290.PmtError,Tr101290.PidError,     Tr101290.TransportError,Tr101290.CrcError
		             ,Tr101290.PcrRepetitionError,Tr101290.PcrDiscontinuityIndicatorError
		             ,Tr101290.PcrAccuracyError,Tr101290.PtsError,Tr101290.CatError


		             ,Tr101290.NitActualError,Tr101290.NitOtherError,Tr101290.SiRepetitionError
		             ,Tr101290.UnreferencedPid,Tr101290.SdtActualError,Tr101290.SdtOtherError
		             ,Tr101290.EitActualError,Tr101290.EitOtherError,Tr101290.EitPfError
		             ,Tr101290.RstError,Tr101290.TdtError);
    return true;	

}
bool C9100Pro::GetPSI(char *buf)
{
	struPat m_patstruct=g_pSerTSParser->GetPat();
	if(0==m_patstruct.section_length)
	{
		return false;
	}

	char patbuf[100]={0};
	sprintf(patbuf,"M%d,%d,%d,%d",m_patstruct.table_id,m_patstruct.section_syntax_indicator,m_patstruct.section_length,m_patstruct.transport_stream_id);


	if(m_patstruct.ProgQuant>0)
	{
		char buftmp[20]={0};
		for(int i=0; i<10; i++) //填满10组
		{
			if(i<m_patstruct.ProgQuant)
			{
				m_patstruct.ProgNum[i];
				m_patstruct.Pid[i];
				sprintf(buftmp,",%d,%d",m_patstruct.ProgNum[i],m_patstruct.Pid[i]);
			}
			else
			{

				sprintf(buftmp,",n,n");
			}

			strcat(patbuf,buftmp);
		}


		int iPMTNum = g_pSerTSParser->GetPmtQuant();

		if(0==iPMTNum)
		{
			return false;
		}
        char pmtbuf[500]={0};

		StruPmt *m_pmtstruct = g_pSerTSParser->GetPmt();
		char pmtbuftmp[60]={0};

		for(int i=0;i<10/*iPMTNum*/;i++)//固定填10个
		{
            if(i<iPMTNum)
            {
                if(m_pmtstruct[i].HasParsed)
                {

                    char patkey1[50]={0};
                    if(m_pmtstruct[i].stream_type[0]>=0x1f)
                    {
                        if(m_pmtstruct[i].stream_type[0]<0x7f)
                        {
                            strcpy(patkey1,"Reserved");
                        }
                        else if(0x7f == m_pmtstruct[i].stream_type[0])
                        {
                            strcpy(patkey1,"IPMP stream");
                        }
                        else
                        {
                            strcpy(patkey1,"User Private");
                        }
                    }
                    else{
                        strcpy(patkey1,PMT_STREAM_TYPE[m_pmtstruct[i].stream_type[0]]);
                    }

                    char patkey2[50]={0};
                    if(m_pmtstruct[i].stream_type[1]>=0x1f)
                    {
                        if(m_pmtstruct[i].stream_type[1]<0x7f)
                        {
                            strcpy(patkey2,"Reserved");
                        }
                        else if(0x7f == m_pmtstruct[i].stream_type[1])
                        {
                            strcpy(patkey2,"IPMP stream");
                        }
                        else
                        {
                            strcpy(patkey2,"User Private");
                        }
                    }
                    else{
                        strcpy(patkey2,PMT_STREAM_TYPE[m_pmtstruct[i].stream_type[1]]);
                    }
                    //有重复
                    sprintf(pmtbuftmp,",%d,%d,%d,%s,%d,%d,%d,%s,%d,%d,%d",m_pmtstruct[i].program_number,m_pmtstruct[i].Pid
                            ,m_pmtstruct[i].ElementQuant,patkey1,m_pmtstruct[i].stream_type[0],m_pmtstruct[i].elementary_PID[0],m_pmtstruct[i].ES_info_length[0]
                            ,patkey2,m_pmtstruct[i].stream_type[1],m_pmtstruct[1].elementary_PID[1],m_pmtstruct[i].ES_info_length[1]);
                }
                else
                {
                    strcpy(pmtbuftmp,",n,n,n,n,n,n,n,n,n,n,n");
                }
            }
            else
            {
                strcpy(pmtbuftmp,",n,n,n,n,n,n,n,n,n,n,n");
            }
            strcat(pmtbuf,pmtbuftmp);
		}
        strcat(buf,patbuf);
        strcat(buf,pmtbuf);
	}
	else
	{
	  return false;
	}

    return  true;

}
bool C9100Pro::GetIPONMIT(char *buf)//N
{

	struMit_Service_List m_SLstruct=g_pSerTSParser->GetMitServicelist();
	if(m_SLstruct.nServicelist > 0)
	{
		char mitbuf[100]={0};
		sprintf(buf,"N%d",m_SLstruct.nServicelist);
		for(int i = 0 ; i < m_SLstruct.nServicelist ; i++)
		{

			struct in_addr tmpAddr;
			tmpAddr.s_addr = m_SLstruct.udp_ipaddress[i];
			tmpAddr.s_addr=htonl(tmpAddr.s_addr);

			sprintf(mitbuf,",%s,%d,%d,%d",inet_ntoa(tmpAddr),m_SLstruct.udp_port[i],m_SLstruct.service_id[i],m_SLstruct.transport_stream_id[i]);
            strcat(buf,mitbuf);
        }

	}
	else
	{
		sprintf(buf,"N0,");
		printf("N0\n");
	}
	return  true;
}
bool C9100Pro::Get4445(char *buf ,int &len)
{
	buf[0]='K';//数据前面加前缀
	int ndiv1=sizeof(double)*DRAW_MDI_NUM;
	memcpy(buf+1,g_pSerTSParser->GetIpMdiDf(),ndiv1);


	int ndiv2=sizeof(u32)*DRAW_MDI_NUM;
	memcpy(buf+1+ndiv1,g_pSerTSParser->GetIpMLR(),ndiv2);


	int ndiv3=sizeof(double)*DRAW_MDI_NUM*2;
	memcpy(buf+1+ndiv1+ndiv2,g_pSerTSParser->GetRtpJitter(),ndiv3);

	double *pmdiDF=g_pSerTSParser->GetIpMdiDf();

	u32     *pmdimlr=g_pSerTSParser->GetIpMLR();
	double *pipJitter=g_pSerTSParser->GetRtpJitter();
	if(pmdiDF==NULL||pmdimlr==NULL||pipJitter==NULL) return false;
	double idealdf=g_pSerTSParser->GetIpMdiDf_Expect();
	double maxdf=g_pSerTSParser->GetDFMax();
	double mindf=g_pSerTSParser->GetDFMin();

	u32     mdimlt =g_pSerTSParser->GetIpMLT();
	//音视频速率准备放到别的地方
	//struRatioInfo sr=g_pSerTSParser->GetRatioInfo();
	//StruTsRate sTT=g_pSerTSParser->GetTSRate();

	//float audioratio=sTT.CurrentRate*sr.AudioRatio/1024;//kbps
	//float videoratio=sTT.CurrentRate*sr.VideoRatio/1024;

	sprintf(buf+1+ndiv1+ndiv2+ndiv3,"%d,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%d,%d",g_pSerTSParser->IptvLoss.AllPac,*pmdiDF,maxdf,mindf
			,idealdf,*pipJitter,*pmdimlr,mdimlt);
    len=1+ndiv1+ndiv2+ndiv3+strlen(buf+1+ndiv1+ndiv2+ndiv3);
    //data 可以把数据一并传过去但是先测试一下
	/*int ndiv4=sizeof(double);
	double dideadf=g_pSerTSParser->GetIpMdiDf_Expect();
	memcpy(buf+1+ndiv1+ndiv2+ndiv3,&dideadf,ndiv4);

	int ndiv5=sizeof(double);
	double dmaxdf=g_pSerTSParser->GetDFMax();
	memcpy(buf+1+ndiv1+ndiv2+ndiv3+ndiv4,&dmaxdf,ndiv5);

	int ndiv6=sizeof(double);
	double dmindf=g_pSerTSParser->GetDFMin();
	memcpy(buf+1+ndiv1+ndiv2+ndiv3+ndiv4+ndiv5,&dmindf,ndiv6);

	int ndiv7=sizeof(int);
	int nmlt=g_pSerTSParser->GetIpMLT();
	memcpy(buf+1+ndiv1+ndiv2+ndiv3+ndiv4+ndiv5+ndiv6,&nmlt,ndiv7);

	len=1+ndiv1+ndiv2+ndiv3+ndiv4+ndiv5+ndiv6+ndiv7;*/
    return true;
}
bool C9100Pro::Get4445Detail(char *buf) //不用啦
{
    double *pmdiDF=g_pSerTSParser->GetIpMdiDf();

    u32     *pmdimlr=g_pSerTSParser->GetIpMLR();
    double *pipJitter=g_pSerTSParser->GetRtpJitter();
    if(pmdiDF==NULL||pmdimlr==NULL||pipJitter==NULL) return false;
    double idealdf=g_pSerTSParser->GetIpMdiDf_Expect();
    double maxdf=g_pSerTSParser->GetDFMax();
    double mindf=g_pSerTSParser->GetDFMin();

    u32     mdimlt =g_pSerTSParser->GetIpMLT();
	struRatioInfo sr=g_pSerTSParser->GetRatioInfo();
	StruTsRate sTT=g_pSerTSParser->GetTSRate();

	float audioratio=sTT.CurrentRate*sr.AudioRatio/1024;//kbps
	float videoratio=sTT.CurrentRate*sr.VideoRatio/1024;

	sprintf(buf,"I%d,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%d,%d,%5.2f,%5.2f",g_pSerTSParser->IptvLoss.AllPac,*pmdiDF,maxdf,mindf
                                   ,idealdf,*pipJitter,*pmdimlr,mdimlt,audioratio,videoratio);


    return true;

}
bool C9100Pro::GetPCR(char *buf ,int &len)
{
	struPcr *sPcr=g_pSerTSParser->GetPcr(0);
	if(!sPcr) return false;
    buf[0]='L';//数据前面加前缀
	int ndiv1=sizeof(double)*DRAW_PCR_NUM;
	memcpy(buf+1,sPcr->Ac,ndiv1);


	int ndiv2=sizeof(double)*DRAW_PCR_NUM;
	memcpy(buf+1+ndiv1,sPcr->Interval,ndiv2);

    //data
	double dcurac=sPcr->CurrentAc;
    double dmaxac=sPcr->MaxAc;
    double dminac=sPcr->MinAc;
    double davgac=sPcr->AvrAc;
	double dcuri=sPcr->CurrentInterval;
	double dmaxi=sPcr->MaxInterval;
	double dmini=sPcr->MinInterval;
	double davgi=sPcr->AvrInterval;

    sprintf(buf+1+ndiv1+ndiv2,"%G,%G,%G,%G,%G,%G,%G,%G",dcurac,dmaxac,dminac,davgac
            ,dcuri,dmaxi,dmini,davgi);
    len=1+ndiv1+ndiv2+strlen(buf+1+ndiv1+ndiv2);

    return true;
}


void C9100Pro::SetCurIptvList(char *pSrcIp,char *pSrcPort,char *pDesIp,char *pDesPort,char *pTransPro)
{

	listtmp.SourceIp=inet_addr(pSrcIp);
	listtmp.SourceIp=ntohl(listtmp.SourceIp);
	listtmp.DestIp=inet_addr(pDesIp);
	listtmp.DestIp=ntohl(listtmp.DestIp);

	listtmp.SourcePort=atoi(pSrcPort);
	listtmp.DestPort=atoi(pDesPort);

	if(*pTransPro=='u'||*pTransPro=='U')
		{
		listtmp.IptvType=UdpIptv;

	}
	else if(*pTransPro=='r'||*pTransPro=='R')
		{
		listtmp.IptvType=RtpIptv;

	}
	else
		{
		listtmp.IptvType=TcpIptv;

	}
}
bool C9100Pro::GetMosDetail(char *buf)
{
    struMos sVmos=g_pSerTSParser->GetMosVideo(0);

    float f_epsnr=g_pSerTSParser->GetEpsnrEnc(0);
    float f_epsnrAtis=g_pSerTSParser->GetEpsnrAtis(0);
    float f_epsnrtran=g_pSerTSParser->GetEpsnrTrans(0);
    struMos sAmos=g_pSerTSParser->GetMosAudio(0);

    sprintf(buf,"G%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,",
            sVmos.MosCur,sVmos.MosMax,sVmos.MosMin,sVmos.BelowRate*100,
            f_epsnrtran,f_epsnr,f_epsnrAtis,
            sAmos.MosCur,sAmos.MosMax,sAmos.MosMin,sAmos.BelowRate*100);
    return true;

}
bool C9100Pro::GetMainFactor(char *buf)
{

	struMos Mosv=g_pSerTSParser->GetMosVideo(0);
	double *pmdiDF=g_pSerTSParser->GetIpMdiDf();
	u32     *pmdimlr=g_pSerTSParser->GetIpMLR();

	struPcr *sPcr=g_pSerTSParser->GetPcr(0);
	StruTr101290  Tr101290;
	Tr101290=g_pSerTSParser->GetTr101290();


	if(!sPcr) return false;
	if(!pmdiDF) return false;
	if(!pmdimlr) return false;


	struIptvLoss sI=g_pSerTSParser->GetIptvParam();

	sprintf(buf,"J%5.2f,%5.2f,%d,%5.2f,%d,%d,%d,%d,",
			Mosv.MosCur,*pmdiDF,*pmdimlr,sPcr->CurrentInterval,
			Tr101290.PatError,Tr101290.SyncByteError,sI.OdQt,sI.DplQt);
	return true;
}
bool C9100Pro::GetSavedMB(char *buf)
{

    sprintf(buf,"H%d",g_pSerTSParser->fsavedMB/1024/1024);
    return true;
}
bool C9100Pro::GetVideoBuffer(char *buf)
{
   return true;
}
bool C9100Pro::GetTsratioTime(char *buf)
{
    Tsratiotime mTsratiotime=g_pSerTSParser->GetTsratiotime();
    if(mTsratiotime.flagenable==false)
    {
        sprintf(buf,"I0,%5.2f,%5.2f,%5.2f,",mTsratiotime.curtsratio,mTsratiotime.curvideoratio,mTsratiotime.curaudioratio);
    }
    else
    {
        sprintf(buf,"I1,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,%5.2f,"
                ,mTsratiotime.curtsratio,mTsratiotime.curvideoratio,mTsratiotime.curaudioratio
                ,mTsratiotime.maxtsratio,mTsratiotime.mintsratio,mTsratiotime.pertsratio
                ,mTsratiotime.maxvideoratio,mTsratiotime.minvideoratio,mTsratiotime.pervideoratio
                ,mTsratiotime.maxaudioratio,mTsratiotime.minaudioratio,mTsratiotime.peraudioratio);
    }

    return  true;
}
bool C9100Pro::Get126(char *buf)
{
    int i15=-1;
    int i1h=-1;
    int i24h=-1;
    struTr126 *Tr126Param15m=g_pSerTSParser->GetTr126(0);//0, 15m  1,1h   ,2,24h
    struTr126 *Tr126Param1h=g_pSerTSParser->GetTr126(1);
    struTr126 *Tr126Param24h=g_pSerTSParser->GetTr126(2);
    if(Tr126Param15m->Passed)
        i15=1;
    else
        i15=0;
    if(Tr126Param1h->Passed)
        i1h=1;
    else
        i1h=0;
    if(Tr126Param24h->Passed)
        i24h=1;
    else
        i24h=0;
    sprintf(buf,"R%4.2f/%4.2f,%4.2f,%d,%d,%4.2f/%4.2f,%4.2f,%d,%d,%4.2f/%4.2f,%4.2f,%d,%d"
              ,Tr126Param15m->CurrentJitter,Tr126Param15m->MaxJitter,Tr126Param15m->LostRate,Tr126Param15m->CurLostQuant,i15
              ,Tr126Param1h->CurrentJitter,Tr126Param1h->MaxJitter,Tr126Param1h->LostRate,Tr126Param1h->CurLostQuant,i1h
              ,Tr126Param24h->CurrentJitter,Tr126Param24h->MaxJitter,Tr126Param24h->LostRate,Tr126Param24h->CurLostQuant,i24h);

    return  true;
}
bool C9100Pro::GetEsMain(char *buf)
{
    char cStreamtype[10]={0};
    float  ffps;//帧率
    short gopnum;//gop帧长
    char cGOP[100]={0};
    int  iPackTotalLoss=0;
	unsigned long long   iPacketRecv=0;
    struAllGop *psAllGop=g_pSerTSParser->GetGopInfo(0);
    struViedoParam *pViedoParam=g_pSerTSParser->GetVideoParam(0);

    if(!psAllGop) return false;
    if(!pViedoParam) return false;

    gopnum=psAllGop->CurGop.FrameQuant;

    struVideo sVideo=g_pSerTSParser->GetVideo(0);
    switch(sVideo.StreamType)
    {
        case 27:
            strcpy(cStreamtype,"H.264");
            break;
        case 2:
            strcpy(cStreamtype,"MPEG-2");
            break;
        case 0x24:
            strcpy(cStreamtype,"H.265");
            break;
        default:
            strcpy(cStreamtype,"---");
            break;
    }
    if(sVideo.vertical_size==1088)
        sVideo.vertical_size=1080;
    //ui->labelFrameWidth->setText(QString::number(sVideo.horizontal_size));
    //ui->labelFrameHeight->setText(QString::number(sVideo.vertical_size));

    if(sVideo.StreamType==0x24)
        ffps=sVideo.FrameRate;
    else if(sVideo.StreamType==27||sVideo.StreamType==2)
    {
        if(sVideo.frame_rate_code==3)
            ffps=25.0;
        else
            ffps=30.0;
    }

    ////////gop帧
    for(int i=0;i<gopnum;i++)
    {
        if(sVideo.StreamType==2)
        {
            switch(psAllGop->CurGop.FrameType[i])
            {
                case 1:
                    cGOP[i]='I';
                    break;
                case 2:
                    cGOP[i]='P';
                    break;
                case 3:
                    cGOP[i]='B';
                    break;

            }

        }
        else if(sVideo.StreamType==0x1b)
        {
            switch(psAllGop->CurGop.FrameType[i])
            {
                case 2:
                    cGOP[i]='I';
                    break;
                case 0:
                    cGOP[i]='P';
                    break;
                case 1:
                    cGOP[i]='B';
                    break;

            }
        }
        else if(sVideo.StreamType==0x24)
        {
            switch(psAllGop->CurGop.FrameType[i])
            {
                case 2:
                    cGOP[i]='I';
                    break;
                case 0:
                    cGOP[i]='P';
                    break;
                case 1:
                    cGOP[i]='B';
                    break;

            }
        }
    }
    iPackTotalLoss=pViedoParam->IFrame.PacLost+pViedoParam->PFrame.PacLost+pViedoParam->BFrame.PacLost+
    pViedoParam->SiFrame.PacLost+pViedoParam->SpFrame.PacLost;

	iPacketRecv=pViedoParam->IFrame.PacQuant+pViedoParam->PFrame.PacQuant+pViedoParam->BFrame.PacQuant+
				pViedoParam->SiFrame.PacQuant+pViedoParam->SpFrame.PacQuant;
    //把si 与 i 融合到一起  sp 与 p 融合到一起
    sprintf(buf,"P%d,%d,%5.1f,%s,%d,%s,",sVideo.horizontal_size,sVideo.vertical_size,ffps,cStreamtype,gopnum,cGOP);

    char bufFramQu[100]={0};
    sprintf(bufFramQu,"%d,%d,%d,",pViedoParam->IFrame.FrameQuant+pViedoParam->SiFrame.FrameQuant,
            pViedoParam->BFrame.FrameQuant,pViedoParam->PFrame.FrameQuant+pViedoParam->SpFrame.FrameQuant);

    char bufPaQu[100]={0};
    sprintf(bufPaQu,"%llu,%llu,%llu,",pViedoParam->IFrame.PacQuant+pViedoParam->SiFrame.PacQuant,
            pViedoParam->BFrame.PacQuant,pViedoParam->PFrame.PacQuant+pViedoParam->SpFrame.PacQuant);

    char bufPaLoss[100]={0};
    sprintf(bufPaLoss,"%d,%d,%d,",pViedoParam->IFrame.PacLost+pViedoParam->SiFrame.PacLost,
            pViedoParam->BFrame.PacLost,pViedoParam->PFrame.PacLost+pViedoParam->SpFrame.PacLost);

    char bufTotalQu[100]={0};
    sprintf(bufTotalQu,"%llu,%d",iPacketRecv,iPackTotalLoss);


    strcat(buf,bufFramQu);
    strcat(buf,bufPaQu);
    strcat(buf,bufPaLoss);
    strcat(buf,bufTotalQu);
    return  true;
}
