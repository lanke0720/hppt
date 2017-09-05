#include "httpd.h"
#include <set>
#include "data_pool.h"
void usage(char* str)
{
    printf("error \n...[IP] [PORT]");
}


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        usage(argv[0]);
        return 1;
    }

    int listen_scok = startup(argv[1], atoi(argv[2]));
    data_pool pool(64);
    set<int> set;
    while(1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);

        int new_sock = accept(listen_scok, (struct sockaddr*)&client, &len);
        if (new_sock < 0)
        {
            perror("accept");
            continue;
        }
            
        printf("get a new client, ip is %s, port is %d\n",\
               inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        handler_request((void*)new_sock, set, pool);
        //handler_request(new_sock, set, pool);
       // handler_request(new_sock, set);
       // pthread_t id;
       // pthread_create(&id, NULL,handler_request, (void*)myset);
       // pthread_detach(id);
    }
    return 0;
}
