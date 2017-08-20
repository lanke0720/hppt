#include <stdlib.h>
#include <iostream>
#include "udpClient.h"
#include "dataType.h"
#include "window.h"
#include "signal.h"
using namespace std;

static void usage(std::string proc)
{
    cout<<"Usage:"<<proc<<"[remote_ip] [remote_port]"<<endl;
}

window win;//全局静态区

std::string nick_name;
std::string school;
dataType sendData;
dataType recvData;
udpClient *sig_cli = NULL;

static void*  run_header(void* arg)
{
    std::string msg = "Welcome To Chat System!";
    int step = 1;
    while(1){
    win.createHeader();
    int h = 0;
    int w = 0;
    //获得窗口的最大的坐标
    getmaxyx(win.header,h,w); 
    //把数据放到窗口
    win.putStringToWin(win.header,h/2, step++,msg);
    //刷新窗口
    win.wrefreshMyWindow(win.header);
    win.clrNumsLines(win.header,h/2,1);
    step%=w;
    step = (step==0 ? 1 : step);
    usleep(300000);
 //   std::string str;
 //   win.getStringFromWin(win.input,str);

 //   getmaxyx(win.output,h,w); 
 //   win.putStringToWin(win.output,1,1,str);
 //   win.wrefreshMyWindow(win.output);
    }

}
static void*  run_output_friend(void* arg)
{
    udpClient *_cli = (udpClient*)arg;
    int point = 1;
    win.createoutput();
    win.createfriendList();
    for(;;){
        int fpoint = 1;
        win.wrefreshMyWindow(win.output);
        win.wrefreshMyWindow(win.friend_list);

        //收数据
        std::string str;
        _cli->recvData(str);
        recvData.stringToValue(str);
        std::string tips = recvData.nick_name;
        tips += "-";
        tips += recvData.school;

        std::string newFriend = tips;
        int h,w;

        if(recvData.cmd == "QUIT")
        {
        _cli->delUser(newFriend);
        }
        else
        {
        tips += "# ";
        tips += recvData.msg;

        getmaxyx(win.output,h,w); 
        win.putStringToWin(win.output,point++,1,tips);
        _cli->addUser(newFriend);
        }
        getmaxyx(win.friend_list,h,w); 
        win.clrNumsLines(win.friend_list,1,h-1);
        win.createfriendList();

        std::vector<std::string>::iterator iter = (_cli->flist).begin();
        for(;iter !=(_cli->flist).end();iter++){
        win.putStringToWin(win.friend_list,fpoint++,1,*iter);
        }

        win.wrefreshMyWindow(win.output);
        win.wrefreshMyWindow(win.friend_list);
        point %= (h-1);
        if(point==0)
        {        
        win.clrNumsLines(win.output,1,h-1);
        win.createoutput();
        win.wrefreshMyWindow(win.output);
        point=1;
        }
        usleep(100000);

    }
}
//输入窗口
static void*  run_input(void* arg)
{
    udpClient *_cli = (udpClient*)arg;
    for(;;){
    win.createinput();
    std::string tips = "Please Enter:";
    int h ,w;
    getmaxyx(win.input,h,w);
    win.putStringToWin(win.input,1,1,tips);
    win.wrefreshMyWindow(win.input);
    std::string str;
    win.getStringFromWin(win.input,str);
    sendData.nick_name = nick_name;
    sendData.school = school;
    sendData.msg = str;
    sendData.cmd = "None";
    
    std::string sendString;
    sendData.valueToString(sendString);
    _cli->sendData(sendString);
    //发完之后清空
    win.clrNumsLines(win.input,1,1);
    usleep(200000);
}
}

void sendQuit(int sig)
{
    sendData.nick_name = nick_name;
    sendData.school = school;
    sendData.msg = "None";
    sendData.cmd = "QUIT";
    std::string sendString;
    sendData.valueToString(sendString);
    exit(0);
}


//./client 125.0.0.1 8080 nick_name school
int main(int argc,char *argv[])
{
    std::cerr<<"-----------------------------"<<std::endl;
    // fflush(stderr);
    if( argc!=5 )
    {
        usage(argv[0]);
        exit(1);
    }
    int port = atoi(argv[2]);
    udpClient cli(argv[1],port);

    sig_cli = &cli;

    //退出信号发给进程
    struct sigaction act;
    act.sa_handler = sendQuit;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(2,&act,NULL);

    nick_name = argv[3];
    school = argv[4];
    cli.initClient();
    
    pthread_t top;
    pthread_t middle;
    pthread_t buttom;

    //维护窗口 头 中部 下部
   pthread_create(&top,NULL,run_header,NULL);
   pthread_create(&middle,NULL,run_output_friend,(void*)&cli);
   pthread_create(&buttom,NULL,run_input,(void*)&cli);

   pthread_join(top,NULL);
   pthread_join(middle,NULL);
   pthread_join(buttom,NULL);

    while(1)
    {
        //std::string sendString;
        //std::string sendString;
        dataType data;
        data.nick_name = "fly pig";
        data.school = "SUST";
        data.msg = "hello world";
        data.cmd = "None";
        //data.valueToString(sendString);
        
        //cli.sendData(sendString);
        cout<<"send done..."<<endl;
        sleep(1);
        //cli.recvData(recvString);
        //cout<<"server echo:"<<recvString<<endl;
    }
    return 0;
}

