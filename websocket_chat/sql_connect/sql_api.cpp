#include "sql_api.h"
sqlApi::sqlApi(const string &_h,\
               const int &_port,\
               const string &_u,\
               const string &_p,\
               const string &_db)
{
    host = _h;
    user = _u;
    passwd = _p;
    db = _db;
    port = _port;
    res = NULL;
    conn = mysql_init(NULL);//#初始化开始进行连接
}

int sqlApi::connect()
{
    //连接到MySQL服务器。
    if(mysql_real_connect(conn, host.c_str(), user.c_str(), passwd.c_str(),db.c_str() , port, NULL, 0))
    //if(mysql_real_connect(conn, host.c_str(), user.c_str(), "nihao",db.c_str() , port, NULL, 0))
    {
        cout<<"connect success!"<<endl;       
    }
    else
    {
       cout<<"connect error!"<<endl;
    }
}

       int sqlApi::insert(const string &_name,\
                          const string &_sex,\
                          const string &_age,\
                          const string &_hobby,\
                          const string &_school)
       {
           string sql = "insert into users (name,sex,age,hobby,school) VALUE ('";
           sql += _name;
           sql += "','";
           sql += _sex;
           sql += "','";
           sql += _age;
           sql += "','";
           sql += _hobby;
           sql += "','";
           sql += _school;
           sql += "')";

           cout<<sql.c_str()<<endl;
           int ret = mysql_query(conn, sql.c_str());//执行sql语句
           if (ret != 0)
           {
               cout<<"insert error!"<<endl;
           }
       }

       int sqlApi::select()
       {
           string sql = "select * from users";
           if (mysql_query(conn, sql.c_str()) == 0)
           {
               res = mysql_store_result(conn);
               if(res)
               {
                   int row = mysql_num_rows(res);
                   int col = mysql_num_fields(res);
                   cout<<"rows: "<<row<<"col: "<<endl;
                   //该结构包含关于字段的信息，如字段名、类型和大小。这里详细介绍了其成员。通过重复调用mysql_fetch_field()，可为每个字段获得MYSQL_FIELD结构。字段值不是该结构的组成部份，它们包含在MYSQL_ROW结构中。 
                   MYSQL_FIELD *fd;

                   //返回采用MYSQL_FIELD结构的结果集的列;
                   for(; fd = mysql_fetch_field(res); )
                   {
                       cout<<fd->name<<"   ";
                   }
                   cout<<endl;
                   int i = 0;
                   int j = 0;
                   for(; i < row; i++)
                   {
                       MYSQL_ROW row_res = mysql_fetch_row(res);
                       j = 0;
                       for(; j < col; j++)
                       {
                           cout<<row_res[j]<<',';
                       }
                       cout<<endl;
                   }
                   cout<<endl;
               }
           }
       }

sqlApi::~sqlApi()
{
    mysql_close(conn);
}

