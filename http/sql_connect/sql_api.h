#ifndef _SQL_API_H_
#define _SQL_API_H_

#include <iostream>
#include <string>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>
using namespace std;
//sql API接口
class sqlApi
{
public:
    //
    sqlApi(const string &_h,\
           const int &_port,\
           const string &_u="root",\
           const string &_p="nihao",\
           const string &_db="httpd");

    int connect();
    int insert(const string &_name,\
               const string &_sex,\
               const string &_age,\
               const string &_hobby,\
               const string &_school);
    int select();
    ~sqlApi();
    
private:
    sqlApi();
    MYSQL *conn;//要连接MYSQL，必须建立MYSQL实例，通过mysql_init初始化方能开始进行连接
    MYSQL_RES *res;//这个结构代表返回行的一个查询的(SELECT, SHOW, DESCRIBE, EXPLAIN)的结果。返回的数据称为“数据集”。 从数据库读取数据，最后就是从MYSQL_RES中读取数据。
    string host;
    string user;
    string passwd;
    string db;
    int port;
};

#endif
