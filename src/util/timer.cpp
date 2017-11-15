#include <util/timer.h>

namespace sogou
{

Timer::Timer() : m_cost(0), m_totalCost(0)
{
}

void Timer::start()
{
    gettimeofday(&m_start, NULL);
}

void Timer::stop()
{
    gettimeofday(&m_end, NULL);
    m_cost = 1000000 * (m_end.tv_sec - m_start.tv_sec) + m_end.tv_usec - m_start.tv_usec;
    m_totalCost += m_cost;
}

int Timer::costTime()
{
    return m_cost;
}

int Timer::totalCostTime()
{
    return m_totalCost;
}
}