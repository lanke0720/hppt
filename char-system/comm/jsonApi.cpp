#include "jsonApi.h"
#include <string>

jsonApi::jsonApi()
{

}

int jsonApi::serialize(const Json::Value &val,std::string &outString)
{
#ifdef _FAST_
    Json::FastWriter w;
    outString = w.write(val);
#else
    Json::StyledWriter w;
    outString = w.write(val);
#endif
    return 0;
}
int jsonApi::deSerialize(std::string &inString,Json::Value &val)
{
    Json::Reader r;
    if(r.parse(inString,val,false)<0)
    {
    return 0;
    }
    return 1;
}
jsonApi::~jsonApi()
{

}

