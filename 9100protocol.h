#ifndef __9100PROTOCOL_H__
#define	__9100PROTOCOL_H__
#include "9100protocol.h"
#include "parser/TSParser.h"
#define  MAX_IPTVLIST 10
struct sStbIptvList
{
    char SrcIP[50];
    int srcPort;//char SrcPort[50];
    char DesIP[50];
    int desPort;//char DesPort[50];
    char Protol[50];
    float speed;//char Speed[50];
};
struct sStbMonitorData
{
    float ipRate;
    float delayFactor;
    float packetLostRate;
    int packetLostCount;
    float videoMos;
    float audioMos;
    int igmpSwitchTime;
    float videoRate;
    float audioRate;

    int allPacketNumber;
    int wholeLossNumber;
    int outSequencePacketNumber;
    int duplicationPacketNumber;

};

class C9100Pro
{
public:
	C9100Pro(void);
	~C9100Pro(void);


    sStbIptvList StbIptvList[MAX_IPTVLIST];
    sStbMonitorData StbMonitorData[MAX_IPTVLIST];
    void InitSnifferAndAnalyzer();
    void DelSnifferAndAnalyzer();


	/////���纯��
    int StartSniffer(char *c_in);
	int StartJSHTSniffer(char *c_in);
    void StartAnalyzer();
    void StartTcpAnalyzer();
    void StopAnalyzer();

    void StartDec();
    void StopDec();
	void StartIGMP();
	void StopIGMP();
	void StartHLSHealth();
	void StopHLSHealth();
    void StartHLS();
    void StopHLS();
    void StartHTTP();
    void StopHTTP();

	bool BeginJSHTAnalyzer(int desport,char * desIP,bool budp,bool bjsht);

    void SetCurIptvList(char *pSrcIp,char *pSrcPort,char *pDesIp,char *pDesPort,char *pTransPro);
    void SetIfFilter(char *ptotal,char *pDesIp,char *pDesPort);
    void SetTSName(char *name);
    void SetPcapNameAndBegin(char *name);
	void TSRatioTimePer(char *name);
    void StopSavePcap();
    void GetTcpFill(int &ires);
    bool GetIpNum(int &IpNum);
    bool GetIpList(char *buf);

    bool GetMosV(char *buf);
    bool Get4445(char *buf ,int &len);
    bool Get4445Detail(char *buf);
	bool GetTr290(char *buf);
	bool GetPSI(char *buf);
	bool GetMosDetail(char *buf);
	bool GetPCR(char *buf,int &len);
	bool GetMainFactor(char *buf);
    bool GetSavedMB(char *buf);
	bool GetVideoBuffer(char *buf);
	bool GetTsratioTime(char *buf);//时间片分析
	bool GetIPONMIT(char *buf);
    bool GetEsMain(char *buf);
    bool Get126(char *buf);

    bool GetIpListForStb();
    bool GetMonitorDataForStb();
    bool GetChannelChange(long fleave,long fjoin);
};






















#endif
