#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H__
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<utility>
#include<sys/types.h>
#include<arpa/inet.h>
#include"log.h"
#include"data_pool.h"
#include"dataType.h"
using namespace std;
class udp_server
{
public:
    //udp_server(string& ip, const int port);
    //int initServer();
    int recvData(string &outString);
    int sendData(string &inString, struct sockaddr_in &client);
    int broadcast();
    //~udp_server();

protected:
   // udp_server(const udp_server&);
    void addUser(struct sockaddr_in &perr);
    void delUser(struct sockaddr_in &perr);
    //string _ip;
    //int _port;
    //int _sock;
    map<int, struct sockaddr_in> online_user;
    data_pool pool;
};

#endif
