#include "udp_server.h"

void* pro_data(void *server)
{
    udp_server *ser=(udp_server*)server;
    while(1)
    {
        string str;
        ser->recvData(str);
    }
}
void* con_data(void *server)
{
    udp_server *ser=(udp_server*)server;
    while(1)
    {
        ser->broadcast();
    }
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout<<"please enter ip & port"<<endl;
    }
    daemon(0,0);
    string ip = argv[1];
    int port = atoi(argv[2]);
    udp_server server(ip, port);
    server.initServer();
    
    pthread_t product;
    pthread_t consumer;
    pthread_create(&product, NULL, pro_data, &server);
    pthread_create(&consumer,NULL, con_data, &server);
    pthread_join(product, NULL);
    pthread_join(consumer, NULL);
    return 0;
}
