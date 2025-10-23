#ifndef SENSORWORKERTHREAD_H
#define SENSORWORKERTHREAD_H

#include <thread>
#include <vector>
#include "SensorData.h"

class SensorWorkerThread
{
public:
    SensorWorkerThread();

    ~SensorWorkerThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();

    void threadFunction();

    // void processZeroCalibCmd();
    // void processResetDeviceCmd();
    // void processExceptionCmd();
    // void processSetTareCmd();
    // void processExceptionPreviousCmd();
    // void processExceptionNextCmd();
};

#endif // SENSORWORKERTHREAD_H
