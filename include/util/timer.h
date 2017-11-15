/*************************************************  
Author: Kevin.XU
Date:   2017-10-26
Desc:   计时器
**************************************************/ 

#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <sys/time.h>

namespace sogou
{

class Timer
{
  public:
    Timer();
    void start();
    void stop();
    int costTime();
    int totalCostTime();

  private:
    struct timeval m_start;
    struct timeval m_end;
    int m_cost;      //本次消耗时间，微秒
    int m_totalCost; //总共消耗时间，微秒
};


}

#endif
