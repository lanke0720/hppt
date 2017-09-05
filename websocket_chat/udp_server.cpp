#include "udp_server.h"

#if 0
udp_server::udp_server(string& ip, const int port)
    :_ip(ip)
    ,_port(port)
    ,_sock(-1)
    ,pool(1024)
    {}
    int udp_server::initServer()
    {
        _sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sock < 0)
        {
            //cerr<< socket <<endl;
            print_log("socket error", FATAL);
            return -1;
        }
        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = inet_addr(_ip.c_str());
    
        bind(_sock, (struct sockaddr*)&local, sizeof(local));
        return 0;
    }
#endif
    int udp_server::recvData(string &outString)
    {
        char buf[1024];
        struct sockaddr_in perr;
        socklen_t len = sizeof(perr);
    
        ssize_t s = recvfrom(_sock, buf, sizeof(buf)-1, 0, (struct sockaddr*)&perr, &len);
        if (s > 0)
        {
            buf[s-1] = 0;
            outString = buf;
            
            pool.putData(outString);

            datatype data;
            data.str_to_val(outString);
            if (data.cmd=="QUIT")
            {
                delUser(perr);
            }
            else
            {
                addUser(perr);
            }
        }
        return s;
    }
    void udp_server::addUser(struct sockaddr_in &perr)
    {
        online_user.insert(pair<int, struct sockaddr_in>(perr.sin_addr.s_addr,perr));               
    }
    void udp_server::delUser(struct sockaddr_in &perr)
    {
        map<int, struct sockaddr_in>::iterator iter = online_user.find(perr.sin_addr.s_addr);
        if (iter != online_user.end())
        {
            online_user.erase(iter);
        }

    }
    int udp_server::sendData(string &inString, struct sockaddr_in &client)
    {
        int ret = sendto(_sock, inString.c_str(), inString.size(),\
                         0, (struct sockaddr*)&client, sizeof(client));
        if (ret > 0)
        {
            return 0;
        }
        return -1;
    }
    int udp_server::broadcast()
    {
        string sendstring;
        pool.getData(sendstring);
        map<int, sockaddr_in>::iterator iter = online_user.begin();
        for (;iter != online_user.end();)
        {
            sendData(sendstring, iter->second/*, sizeof(iter->second)*/);
            ++iter;
        }
    }
    udp_server::~udp_server()
    {
        if (_sock > 0)
        {
            close(_sock);
        }
    }
