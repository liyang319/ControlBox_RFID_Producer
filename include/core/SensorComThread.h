#ifndef SENSORCOMTHREAD_H
#define SENSORCOMTHREAD_H

#include <thread>
#include <vector>
#include "SensorWorkerThread.h"

class SensorComThread
{
public:
    SensorComThread();

    ~SensorComThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();
    bool getAddrFromCmd(uint8_t &devAddr, uint8_t *cmdBuf, int len);

    SensorWorkerThread sensorWorkerThread;
    void threadFunction();
};

#endif // SENSORCOMTHREAD_H
