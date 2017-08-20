#ifndef _UDP_SERVER_
#define _UDP_SERVER_

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "log.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include "data_pool.h"
#include <stdlib.h>
#include <pthread.h>
#include "dataType.h"

class udpServer
{
public:
    udpServer(const std::string _ip,const int &_port);
    void initServer();
    int recvData(std::string &outString);
    int sendData(const std::string &inString,\
            struct sockaddr_in &remote,socklen_t &len);
    int brocast();
    ~udpServer();
private:
    udpServer(const udpServer&);
    void addUser(const struct sockaddr_in &remote);
    void delUser(const struct sockaddr_in &remote);
private:
    int sock;
    std::string ip;
    int port;
    std::map<in_addr_t,struct sockaddr_in>  user_list;
    data_pool  pool;
};

#endif
