#include "httpd.h"
//#include "data_pool.h"
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#define GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
static const char encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "abcdefghijklmnopqrstuvwxyz0123456789+/";

void print_log(char *msg, int level)
{
    #ifdef _STDOUT_
    const char * const level_msg[]=
    {
        "SUCCESS",
        "NOTICE",
        "WARNING",
        "ERROR",
        "FATAL",
    };
    printf("[%s][%s]\n", msg, level_msg[level%5]);
    #endif
}

int startup(const char *ip, int  port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        print_log(strerror(errno), FATAL);
        exit(2);
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = inet_addr(ip);
    if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)
    {
        print_log(strerror(errno), FATAL);
        exit(3);
    }

    if(listen(sock, 10) < 0)
    {
        print_log(strerror(errno), FATAL);
        exit(4);
    }
    return sock;
}

//ret > 1, line != '\0', ret=1&line='\n',  ret<=0&&line=='\0'
static int get_line(int sock, char line[], int size)
{
    // read 1 char , one by one
    char c = '\0';
    int len = 0;
    while( c != '\n' && len < size-1)
    {
        int r = recv(sock, &c, 1, 0);
        if(r > 0)
        {
            if(c == '\r')
            {
                //窥探
                int ret = recv(sock, &c, 1, MSG_PEEK);
                if(ret > 0)
                {
                    if(c == '\n')
                    {
                        recv(sock, &c, 1, 0);
                    }
                    else
                    {
                        c = '\n';
                    }
                }
            }// \r->\n \r\n -> \n
            line[len++] = c;
        }
        else
        {
            c = '\n';
        }
    }
    line[len]='\0';
    return len;
}
#if 1 
int base64_encode(char *in_str, int in_len, char *out_str)
{
    BIO *b64, *bio;
    BUF_MEM *bptr = NULL;
    size_t size = 0;

    if (in_str == NULL || out_str == NULL)
        return -1;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in_str, in_len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out_str, bptr->data, bptr->length);
    out_str[bptr->length-1] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}
#endif
#if 0
int base64_encode(const char *in, int in_len,char *out, int out_size)
{
        unsigned char triple[3];
        int i;
        int len;
        int line = 0;
        int done = 0;

    while (in_len)
    {
                len = 0;
        for (i = 0; i < 3; i++) {
            if (in_len) {
                                triple[i] = *in++;
                                len++;
                                in_len--;
                            
            } else
                            triple[i] = 0;
                    
        }

                if (done + 4 >= out_size)
                    return -1;

                *out++ = encode[triple[0] >> 2];
                *out++ = encode[((triple[0] & 0x03) << 4) |
                                                     ((triple[1] & 0xf0) >> 4)];
                *out++ = (len > 1 ? encode[((triple[1] & 0x0f) << 2) |
                                                                 ((triple[2] & 0xc0) >> 6)] : '=');
                *out++ = (len > 2 ? encode[triple[2] & 0x3f] : '=');

                done += 4;
                line += 4;
            
    }

        if (done + 1 >= out_size)
            return -1;

        *out++ = '\0';

        return done;

}
#endif 

int _readline(char* allbuf,int level,char* linebuf)
{
    int len = strlen(allbuf);
    for (;level<len;++level)
    {
        if(allbuf[level]=='\r' && allbuf[level+1]=='\n')
            return level+2;
        else
            *(linebuf++) = allbuf[level];
    }
    return -1;
}

int shakehands(int cli_fd)
{
    int level = 0;
    char buffer[1024];
    char linebuf[256];
    char sec_accept[32];
    unsigned char sha1_data[1025]={0};
    char head[1024] = {0};

    if (read(cli_fd,buffer,sizeof(buffer))<=0)//buffer-> alldata of cli_fd
        perror("read");
    printf("request\n");
    printf("%s\n",buffer);

    do {
        memset(linebuf,0,sizeof(linebuf));
        level = _readline(buffer,level,linebuf);
        printf("line:%s\n",linebuf);

        if (strstr(linebuf,"Sec-WebSocket-Key")!=NULL)
        {
            strcat(linebuf,GUID);
//            printf("key:%s\nlen=%d\n",linebuf+19,strlen(linebuf+19));
             SHA1((unsigned char*)&linebuf+19,strlen(linebuf+19),(unsigned char*)&sha1_data);
//            printf("sha1:%s\n",sha1_data);
            int len_sh1_data = strlen((char*)sha1_data);
           // base64_encode(sha1_data, strlen(sha1_data), sec_accept, sizeof(sec_accept));
            base64_encode((char*)sha1_data, len_sh1_data, sec_accept);
                //            printf("base64:%s\n",sec_accept);
            /* write the response */
            sprintf(head, "HTTP/1.1 101 Switching Protocols\r\n" \
                          "Upgrade: websocket\r\n" \
                          "Connection: Upgrade\r\n" \
                          "Sec-WebSocket-Accept: %s\r\n" \
                          "\r\n",sec_accept);

            printf("response\n");
            printf("%s",head);
            if (write(cli_fd,head,strlen(head))<0)
                perror("write");

            break;
        }
    }while((buffer[level]!='\r' || buffer[level+1]!='\n') && level!=-1);
    return 0;
}


static void echo_string(int sock)
{}

static int echo_www(int sock, char *path, int size)
{
    printf("\n enter echo_www \n");
    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        echo_string(sock);
        print_log(strerror(errno), FATAL);
        return 8;
    }

    const char *echo_line="HTTP/1.0 200 OK\r\n";
    send(sock, echo_line, strlen(echo_line), 0);
    const char *null_line="\r\n";
    send(sock, null_line, strlen(null_line), 0);

    if(sendfile(sock, fd, NULL, size) < 0)
    {
        echo_string(sock);
        print_log(strerror(errno), FATAL);
        return 9;
    }

    printf("\n quit echo_www \n");
    execl(path, path, NULL);
    //close(fd);
    return 0;
}

static void drop_header(int sock)
{
    char line[1024];
    int ret = -1;
    do{
        ret = get_line(sock, line, sizeof(line));
    }while(ret>0 && strcmp(line, "\n"));
}


int recv_frame_head(int fd,frame_head* head)
{
    char one_char;
    /*read fin and op code*/
    if (read(fd,&one_char,1)<=0)
    {
        perror("read fin");
        return -1;
    }
    head->fin = (one_char & 0x80) == 0x80;
    head->opcode = one_char & 0x0F;
    if (read(fd,&one_char,1)<=0)
    {
        perror("read mask");
        return -1;
    }
    head->mask = (one_char & 0x80) == 0X80;

    /*get payload length*/
    head->payload_length = one_char & 0x7F;

    if (head->payload_length == 126)
    {
        char extern_len[2];
        if (read(fd,extern_len,2)<=0)
        {
            perror("read extern_len");
            return -1;
        }
        head->payload_length = (extern_len[0]&0xFF) << 8 | (extern_len[1]&0xFF);
    }
    else if (head->payload_length == 127)
    {
        char extern_len[8],temp;
        int i;
        if (read(fd,extern_len,8)<=0)
        {
            perror("read extern_len");
            return -1;
        }
        for(i=0;i<4;i++)
        {
            temp = extern_len[i];
            extern_len[i] = extern_len[7-i];
            extern_len[7-i] = temp;
        }
        memcpy(&(head->payload_length),extern_len,8);
    }

    /*read masking-key*/
    if (read(fd,head->masking_key,4)<=0)
    {
        perror("read masking-key");
        return -1;
    }

    return 0;
}


int send_frame_head(int fd,frame_head* head)
{
    char *response_head;
    int head_length = 0;
    if(head->payload_length<126)
    {
        response_head = (char*)malloc(2);
        response_head[0] = 0x81;
        response_head[1] = head->payload_length;
        head_length = 2;
    }
    else if (head->payload_length<0xFFFF)
    {
        response_head = (char*)malloc(4);
        response_head[0] = 0x81;
        response_head[1] = 126;
        response_head[2] = (head->payload_length >> 8 & 0xFF);
        response_head[3] = (head->payload_length & 0xFF);
        head_length = 4;
    }
    else
    {
        //no code
        response_head = (char*)malloc(12);
//        response_head[0] = 0x81;
//        response_head[1] = 127;
//        response_head[2] = (head->payload_length >> 8 & 0xFF);
//        response_head[3] = (head->payload_length & 0xFF);
        head_length = 12;
    }

    if(write(fd,response_head,head_length)<=0)
    {
        perror("write head");
        return -1;
    }

    free(response_head);
    return 0;
}


void myumask(char *data,int len,char *mask)
{
    int i;
    for (i=0;i<len;++i)
        *(data+i) ^= *(mask+(i%4));
}

//thread 
void *handler_request(void *arg, set<int> &set, data_pool &poll)
//void *handler_request(int arg, set<int> set)
{
   // pool_user* myset = (pool_user*)arg;
//    int sock = myset->conn;
    int sock = (int)arg;
    /*
    int sock = (int)arg;
    #ifdef _DEBUG_
    char line[1024];
    do{
        int ret = get_line(sock, line, sizeof(line));
        if(ret > 0)
        {
            printf("%s", line);
        }
        else
        {
            printf("request ...... done!\n");
            break;
        }
    }while(1);
    #else
    int ret = 0;
    char buf[SIZE];
    char method[SIZE/10];
    char url[SIZE];
    char path[SIZE];
    int i, j;
    int cgi = 0;
    char *query_string = NULL;
    if(get_line(sock, buf, sizeof(buf)) <= 0)
    {
        echo_string(sock);
        ret = 5;
        goto end;
    }
    i=0;//method ->index
    j=0;//buf -> index

    while( !isspace(buf[j]) &&\
          j < sizeof(buf) &&\
          i < sizeof(method)-1)
    {
        method[i]=buf[j];
        i++, j++;
    }
    method[i] = 0;
     if(strcasecmp(method, "GET") &&\
    strcasecmp(method, "POST") )
    {
    echo_string(sock);
    ret = 6;
    goto end;
}
    if(strcasecmp(method, "POST") == 0)
    {
cgi = 1;
}
//buf -> "GET          /      http/1.0"
    while(isspace(buf[j]) && j < sizeof(buf))
    {
        j++;
    }
    i=0;
    while(!isspace(buf[j]) && j < sizeof(buf) && i < sizeof(url)-1)
    {
        url[i] = buf[j];
        i++, j++;
    }
    url[i] = 0;
    printf("method: %s, url: %s\n", method, url);
    query_string = url;
    while(*query_string != '\0')
    {
        if(*query_string == '?')
        {
            *query_string = '\0';
            query_string++;
            cgi = 1;
            break;
        }
        query_string++;
    }
    sprintf(path, "wwwroot%s", url);
    //method, url, query_string, cgi
    if(path[strlen(path)-1] == '/')
    { // '/'
     strcat(path, "index.html");
    }

    struct stat st;
    if(stat(path, &st) != 0)
    {
        echo_string(sock);
        ret = 7;
        goto end;
    }
    else
    {
        if(S_ISDIR(st.st_mode))
        {
            strcat(path, "/index.html");
        }
        else if( (st.st_mode & S_IXUSR) || \
                (st.st_mode & S_IXGRP) || \
                (st.st_mode & S_IXOTH) )
        {
            cgi=1;
        }
        else
        {
        }
        //ok->cgi=?, path, query_string, method
        if(cgi)
        {
            printf("enter CGI\n");
            //exe_cgi(sock, method, path, query_string);
        }
        else
        {
            printf("method: %s, url: %s, path: %s, cgi: %d, query_string: %s\n", method, url, path, cgi, query_string);
            drop_header(sock); //!!!!!!!!!!!!!!
            echo_www(sock, path, st.st_size);
        }
    }
*/
    
  //  drop_header(sock); //!!!!!!!!!!!!!!
  //  char* path = "wwwroot/index.html"; 
  //  struct stat st;
  //  stat(path, &st);
  //  echo_www(sock, path, st.st_size);
   printf("\n enter shakehands  \n");
   shakehands(sock);
   printf("\n shakehands over \n");
    
    while (1)
    {
        frame_head head;
        int rul = recv_frame_head(sock,&head);
        if (rul < 0)
            break;

        printf("fin=%d\nopcode=0x%X\nmask=%d\npayload_len=%llu\n",head.fin,head.opcode,head.mask,head.payload_length);

        //echo head
        send_frame_head(sock,&head);
        //read payload data
        char payload_data[1024] = {0};
        int size = 0;
        do {
            int rul;
            rul = read(sock,payload_data,1024);
            if (rul<=0)
                break;

            size+=rul;

            myumask(payload_data,size,head.masking_key);
            printf("recive:%s",payload_data);
            
            //echo data
            if (write(sock,payload_data,rul)<=0)
                break;
        }while(size<head.payload_length);
        printf("\n-----------\n");

    }

//end:
    printf("quit client...\n");
    close(sock);
    return (void*)-1;

}



















