/*************************************************************************
	> File Name: data_pool.cpp
	> Author: 
	> Mail: 
	> Created Time: Tue 15 Aug 2017 11:49:15 PM PDT
 ************************************************************************/
#include "data_pool.h"

data_pool::data_pool(int cap)
    :_cap(cap)
    ,_datapoll(cap)
    ,c_step(0)
    ,p_step(0)
    ,_size(0)
    {
        sem_init(&_data, 0, 0);
        sem_init(&_blank,0,cap);
    }

void data_pool::putData(string &instring)
{
    sem_wait(&_blank);
    _datapoll[p_step++] = instring;
    p_step %= _cap;
    _size++;
    sem_post(&_data);
}

void data_pool::getData(string &outstring)
{
    sem_wait(&_data);
    outstring = _datapoll[c_step++];
    c_step %= _cap;
    _size--;
    sem_post(&_blank);
}

data_pool::~data_pool()
{
    _cap = 0;
    _size = 0;
    sem_destroy(&_blank);
    sem_destroy(&_data);
}
