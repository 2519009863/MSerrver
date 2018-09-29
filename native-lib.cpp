
//#include <jni.h>
#include <string>
#include "parser/TSParser.h"
//#include "parser/CallFun.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
//#include <cutils/sockets.h>
//#include <utils/Log.h>
#include "9100net.h"
//#include <android/log.h>
#include <iostream>

using namespace std;

//___________>鎹曡幏娈甸敊璇?
#include <signal.h>
//#include <execinfo.h>

//linux 编译
//g++ native-lib.cpp 9100net.cpp 9100protocol.cpp ./parser/CAtscSiParse.cpp ./parser/CDespParse.cpp ./parser/CDvbSiParse.cpp ./parser/CEsParse.cpp ./parser/CIpParse.cpp ./parser/CIptvParse.cpp ./parser/CParseBase.cpp ./parser/CPesParse.cpp ./parser/CPsiParse.cpp ./parser/CTsBaseParse.cpp
// ./parser/CTsParse.cpp ./parser/TSParser.cpp -lpthread -o tsexe
//同时要去掉 #include <android/log.h>
/*void *buffer[1000];
void fault_trap(int n,struct siginfo *siginfo,void *myact)
{
    int i, num;
    char **calls;
    printf("Fault address:%X\n",siginfo->si_addr);
   // num = backtrace(buffer, 1000);
   // calls = backtrace_symbols(buffer, num);
   // for (i = 0; i < num; i++)
   //     printf("%s\n", calls[i]);
    exit(1);
}
void setuptrap()
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=fault_trap;
    sigaction(SIGSEGV,&act,NULL);
}
//鎵撳嵃鍑哄湴鍧€鍚?閫氳繃 addr2line or arm-none-linux-gnueabi-addr2line 鑾峰緱鍑芥暟鍚?
//鍏跺疄涓嶇敤姝ゅ嚱鏁?涔熷彲浠ョ湅log鏂囦欢 (/var/log/message )
//鏂规硶
//arm-none-linux-gnueabi-addr2line 0x814e5d3 -e DShark -f
//arm-none-linux-gnueabi-addr2line 0x25980 -e /mnt/hgfs/aike/svn/build-IPTV-Qt_for_ARM-Debug/iptv -f
*/
//TCP 检测不到流的原因汇总
//1 协议传的是wlan0 不是 eth0
//2 交换机复位 重新设置 镜像1235 -》4  新班子  老板子 2345>1  一代机 1234》5
//3 目前 检测if(CurDstIp!=IpToParse.DestIp || CurSrcIp!=IpToParse.SourceIp
//          ||CurSrcPt!=IpToParse.SourcePort)
//4 吉视汇通要注意 mfJSHT 写成true; 不给吉视汇通要写成false
//5 //xc save 保存ts文件  //xc play 播放tcp 重新pcr发送
int main(int argc, char* argv[])
{
    //test cjson
    /*int iValue;
    std::string strValue;
    neb::CJsonObject oJson("{\"refresh_interval\":60,"
                                   "\"dynamic_loading\":["
                                   "{"
                                   "\"so_path\":\"plugins/User.so\", \"load\":false, \"version\":1,"
                                   "\"cmd\":["
                                   "{\"cmd\":2001, \"class\":\"neb::CmdUserLogin\"},"
                                   "{\"cmd\":2003, \"class\":\"neb::CmdUserLogout\"}"
                                   "],"
                                   "\"module\":["
                                   "{\"path\":\"im/user/login\", \"class\":\"neb::ModuleLogin\"},"
                                   "{\"path\":\"im/user/logout\", \"class\":\"neb::ModuleLogout\"}"
                                   "]"
                                   "},"
                                   "{"
                                   "\"so_path\":\"plugins/ChatMsg.so\", \"load\":false, \"version\":1,"
                                   "\"cmd\":["
                                   "{\"cmd\":2001, \"class\":\"neb::CmdChat\"}"
                                   "],"
                                   "\"module\":[]"
                                   "}"
                                   "]"
                                   "}");
    std::cout << oJson.ToString() << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << oJson["dynamic_loading"][0]["cmd"][1]("class") << std::endl;
    oJson["dynamic_loading"][0]["cmd"][0].Get("cmd", iValue);
    std::cout << "iValue = " << iValue << std::endl;
    oJson["dynamic_loading"][0]["module"][0].Get("path", strValue);
    std::cout << "strValue = " << strValue << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    oJson.AddEmptySubObject("depend");
    oJson["depend"].Add("nebula", "https://github.com/Bwar/Nebula");
    oJson["depend"].AddEmptySubArray("bootstrap");
    oJson["depend"]["bootstrap"].Add("BEACON");
    oJson["depend"]["bootstrap"].Add("LOGIC");
    oJson["depend"]["bootstrap"].Add("LOGGER");
    oJson["depend"]["bootstrap"].Add("INTERFACE");
    oJson["depend"]["bootstrap"].Add("ACCESS");
    std::cout << oJson.ToString() << std::endl;
    std::cout << "-------------------------------------------------------------------" << std::endl;
    std::cout << oJson.ToFormattedString() << std::endl;*/
    //test

    int g_port=18090;
    //setuptrap();
    C9100Net *Net=new C9100Net();
    Net->TcpServerInit(g_port);
    //setuptrap();
    printf("close sock\n");
    close(Net->sock);
    printf("close snew\n");
    //close(Net->snew);
    printf("over-----\n");

    delete Net;Net=NULL;//xc mem
    return 0;
}
