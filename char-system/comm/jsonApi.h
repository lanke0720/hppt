#ifndef _JSONAPI_H_
#define _JSONAPI_H_

#include <iostream>
#include "json/json.h"
#include <string>

class jsonApi
{
public:
    jsonApi();
    ~jsonApi();
    static int serialize(const Json::Value &val,std::string &outString);
    static int deSerialize(std::string &inString,Json::Value &val);
};

#endif
