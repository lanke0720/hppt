#include "log.h"

static std::string level[] = {
    "[NORMAL]",
    "[NOTICE]",
    "[WARNING]",
    "[FATAL]",
};
void printf_log(std::string msg,int _level)
{

#ifdef DEBUG
    if(_level>=0 && _level<=3)
    {
    std::cout<<level[_level]<<msg<<std::endl;
    }
#endif
}
