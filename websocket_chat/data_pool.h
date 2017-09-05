/*************************************************************************
	> File Name: data_pool.h
	> Author: 
	> Mail: 
	> Created Time: Tue 15 Aug 2017 09:08:28 PM PDT
 ************************************************************************/

#ifndef _data_pool_H_
#define _data_pool_H_
#include <iostream>
#include <vector>
#include <semaphore.h>

using namespace std;
class data_pool
{
public:
    data_pool(int cap);
    void putData(string &instring);
    void getData(string &outstring);
    ~data_pool();
    private:
        int _cap;
    vector<string> _datapoll;
    int c_step;
    int p_step;
    sem_t _data;
    sem_t _blank;
    int _size;
};

#endif
