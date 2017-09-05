/*************************************************************************
	> File Name: http.h
	> Author: 
	> Mail: 
	> Created Time: Fri 21 Jul 2017 10:59:35 PM PDT
 ************************************************************************/

#ifndef _HTTP_H_
#define _HTTP_H_

#include "data_pool.h"
#include<stdio.h>
#include<string.h>
#include<strings.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<fcntl.h>
#include<unistd.h>
#include<syslog.h>
#include <ctype.h>
#include <sys/wait.h>
#include<errno.h>
#include<strings.h>
#include <set>

#define SIZE 1024

#define SUCCESS 0
#define NOTICE  1
#define WARNING 2
#define ERROR   3
#define FATAL   4
typedef struct _frame_head {
    char fin;                      //0x1 || 0x0
    char opcode;                   //文件类型
    char mask;                     //是否有掩码
    unsigned long long payload_length;  //7(125)  || 7+ 16(2_126) || 7+64(8_127)
    char masking_key[4]; //秘钥
}frame_head;


struct pool_user
{
    data_pool pool;
    set<int> users;
    int conn;

    pool_user()
        :pool(64)
    {
    }

};

void usage(char* name);
int startup(const char* ip, int port);

static void drop_header(int sock);
static int get_line(int sock, char line[], int size);
//void *handler_request(void *arg);
void *handler_request(void* arg, set<int>& set, data_pool& poll);
//void *handler_request(int arg, set<int> set);
static int echo_www(int sock, char *path, int size);
static int exe_cgi(int sock, char *method, char* path, char*query_string);
void print_log(char *msg, int level);
#endif
