
#include "co_routine.h"
extern "C"
{
    #include "httpd.h"
}
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <stack>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
struct task_t   //存放一个协程信息（pcb)
{
	stCoRoutine_t *co; //协程控制块
	int fd;            //文件描述符
};

static stack<task_t*> g_readwrite; //首先存的是空任务协程
static int g_listen_fd = -1;

static int SetNonBlock(int iSock) //设置非阻塞
{
    int iFlags;

    iFlags = fcntl(iSock, F_GETFL, 0);
    iFlags |= O_NONBLOCK;
    iFlags |= O_NDELAY;
    int ret = fcntl(iSock, F_SETFL, iFlags);
    return ret;
}
//任务协程
static void *readwrite_routine( void *arg ) //task_t * task
{

	co_enable_hook_sys();

	task_t *co = (task_t*)arg;
	char buf[ 1024 * 16 ];
	for(;;)
	{	//如果协程没有任务可以处理，把自己放入任务队列
		if( -1 == co->fd )
		{
			g_readwrite.push( co ); 
			co_yield_ct();
			continue;
		}

		int fd = co->fd;
		co->fd = -1;	

		for(;;)
		{   //监听fd事件，直到发生
			struct pollfd pf = { 0 };
			pf.fd = fd;
			pf.events = (POLLIN|POLLERR|POLLHUP);
			co_poll( co_get_epoll_ct(),&pf,1,1000);//监听函数
			
			//使用多线程一样，处理接收到的连接
       handler_request(fd);
			/*
			int ret = read( fd,buf,sizeof(buf) );
			if( ret > 0 )
			{
				ret = write( fd,buf,ret );
			}
			if( ret <= 0 )
			{
				close( fd );
				break;
			}
			*/
		}

	}
	return 0;
}
int co_accept(int fd, struct sockaddr *addr, socklen_t *len );
static void *accept_routine( void * )
{
	co_enable_hook_sys();
	printf("accept_routine\n");
	fflush(stdout);
	for(;;)
	{   //确保有任务协程存在，如果没有则一直sleep,直到任务队列不为空
		//printf("pid %ld g_readwrite.size %ld\n",getpid(),g_readwrite.size());
		if( g_readwrite.empty() )
		{
			printf("empty\n"); //sleep
			struct pollfd pf = { 0 };
			pf.fd = -1;
			poll( &pf,1,1000);

			continue;

		}
		struct sockaddr_in addr; //maybe sockaddr_un;
		memset( &addr,0,sizeof(addr) );
		socklen_t len = sizeof(addr);
      
		int fd = co_accept(g_listen_fd, (struct sockaddr *)&addr, &len);
		if( fd < 0 )
		{
			struct pollfd pf = { 0 };
			pf.fd = g_listen_fd;
			pf.events = (POLLIN|POLLERR|POLLHUP);
			co_poll( co_get_epoll_ct(),&pf,1,1000 );
			continue;
		}
		
		if( g_readwrite.empty() )
		{
			close( fd );
			continue;
		}
		SetNonBlock( fd );
		//把监听的fd 发放给任务协程处理
		task_t *co = g_readwrite.top();
		co->fd = fd;
		g_readwrite.pop();
		co_resume( co->co );
	}
	return 0;
}

static void SetAddr(const char *pszIP,const unsigned short shPort,struct sockaddr_in &addr)
{
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(shPort);
	int nIP = 0;
	if( !pszIP || '\0' == *pszIP   
	    || 0 == strcmp(pszIP,"0") || 0 == strcmp(pszIP,"0.0.0.0") 
		|| 0 == strcmp(pszIP,"*") 
	  )
	{
		nIP = htonl(INADDR_ANY);
	}
	else
	{
		nIP = inet_addr(pszIP);
	}
	addr.sin_addr.s_addr = nIP;

}

static int CreateTcpSocket(const unsigned short shPort /* = 0 */,const char *pszIP /* = "*" */,bool bReuse /* = false */)
{
	int fd = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	if( fd >= 0 )
	{
		if(shPort != 0) //port
		{
			if(bReuse)  //端口占用问题
			{
				int op = 1;
				//int nReuseAddr = 1;
				setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&op,sizeof(op)); //重启 
				//setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&nReuseAddr,sizeof(nReuseAddr)); //重启
			}
			struct sockaddr_in local;
			bzero(&local,sizeof(local));
			local.sin_family = AF_INET;
			local.sin_port = htons(shPort);
			local.sin_addr.s_addr = inet_addr(pszIP);
			//struct sockaddr_in addr ;			
			//SetAddr(pszIP,shPort,addr); 
			int ret = bind(fd,(struct sockaddr*)&local,sizeof(local));
			if( ret != 0)
			{
				close(fd);
				return -1;
			}
		}
	}
	return fd;
}

void usage(char* str)
{
    printf("error \n...[IP] [PORT] [cnt] [proccnt]/n");
}

int main(int argc,char *argv[])
{
    if (argc != 5)
    {
        usage(argv[0]);
        return 1;
    }
	//获取控制台参数
	const char *ip = argv[1];
	int port = atoi( argv[2] );
	int cnt = atoi( argv[3] ); //一个进程的协程数目
	int proccnt = atoi( argv[4] ); //创建多少个进程
	// 创建套接字
	g_listen_fd = CreateTcpSocket( port,ip,true ); 	
	listen( g_listen_fd,1024 ); //设置监听
	printf("listen %d %s:%d\n",g_listen_fd,ip,port);

	SetNonBlock( g_listen_fd );//设置非阻塞

	for(int k=0;k<proccnt;k++)
	{
		pid_t pid = fork();
		if( pid > 0 )
		{
			continue;
		}
		else if( pid < 0 )
		{
			break;
		}
		for(int i=0;i<cnt;i++)
		{   //开启任务协程
			task_t * task = (task_t*)calloc( 1,sizeof(task_t) );
			task->fd = -1;

			co_create( &(task->co),NULL,readwrite_routine,task );
			co_resume( task->co ); //开始运行这个协程

		}
		stCoRoutine_t *accept_co = NULL;
		//开启accept协程
		co_create( &accept_co,NULL,accept_routine,0 );
		co_resume( accept_co );
		
		//检测事件发生的主循环函数。该函数中周期地监听是否有事件发生，
		//然后唤醒那些等待特点时间发生的协程
		co_eventloop( co_get_epoll_ct(),0,0 );

		exit(0);
	}
	return 0;
}

