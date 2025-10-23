#ifndef DATAREPORTTHREAD_H
#define DATAREPORTTHREAD_H

#include <thread>
#include <vector>
#include "SensorData.h"

class DataReportThread
{
public:
    DataReportThread();

    ~DataReportThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    int loopIndex;
    int publishInterval;
    void init();

    void threadFunction();
};

#endif // DATAREPORTTHREAD_H
