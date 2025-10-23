#ifndef REPORTWORKERTHREAD_H
#define REPORTWORKERTHREAD_H

#include <thread>
#include <vector>
#include <atomic>
#include "Base.h"
#include "WeighData.h"
#include "DataFormater.h"
#include "DB.h"

class ReportWorkerThread
{
public:
    ReportWorkerThread();

    ~ReportWorkerThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    int loopIndex;
    std::atomic<uint16_t> handleflag;
    // DB weighDataDB;

    void init();

    void threadFunction();

    void getDeviceData();
    void getLocationData();
};

#endif // REPORTWORKERTHREAD_H
