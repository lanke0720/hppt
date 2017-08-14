#include "sql_api.h"

void insertToHttpd_users(const char *string_arg)
{
    char arg_buff[1024];

    char *my_arg[6];//指针数组获取页面所填写的串
    strcpy(arg_buff, string_arg);
    char* start = arg_buff;
    int i = 0;
    while(*start)
    {
        if(*start == '=')
        {
            start++;
            my_arg[i++] = start;
            continue;
        }
        if(*start == '&')
        {
            *start = 0;
        }
        start++;
    }
    my_arg[i] = NULL;
    sqlApi mydb("127.0.0.1", 3306);
    mydb.connect();
    //向表中插入所页面所填写的信息。
    mydb.insert(my_arg[0],my_arg[1],my_arg[2],my_arg[3],my_arg[4]);
    cout<<"Insert OK"<<endl;
}

int main()
{
    cout<<"-------------------------------------"<<endl;
    //读取正文信息
    char* method = NULL;
    char* query_string = NULL;
    char* string_arg = NULL;
    int connect_len = -1;
    char buf[1024];
    if ((method = getenv("METHOD")))
    {
        cout<<"Insert_CGI::METHOD::"<<method<<endl;
        if(strcasecmp(method, "GET") == 0)
        {
            cout<<"~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
            if((query_string=getenv("QUERY_STRING")))
            {
                string_arg = query_string;
                cout<< "Insert_CGI::query_string::"<< string_arg << endl;
            }
        }
        else//post
        {
            cout<<"1233333333333333\n";
            if(getenv("CONTENT_LENGTH"))
            {
                connect_len = atoi(getenv("CONTENT_LENGTH"));
                int i = 0;
                for(; i < connect_len; i++)
                {
                    read(0, &buf[i], 1);
                }
                buf[i] = '\0';
                string_arg = buf;
            }
        }
    }
    cout<< "Insert_CGI"<<string_arg<<endl;
    insertToHttpd_users(string_arg);

    return 0;
}
