#ifndef _DATA_TYPE_H_
#define _DATA_TYPE_H_

#include <iostream>
#include "json/json.h"
#include <string>
#include "jsonApi.h"

#define SIZE 1024

class dataType
{
public:
    //构造Value对象转换成string
    int  stringToValue(std::string &inString);
    int valueToString(std::string &outString);
    dataType();
    ~dataType();
public:
    std::string nick_name;
    std::string school;
    std::string msg;
    //保留字段
    std::string cmd;

};


#endif
