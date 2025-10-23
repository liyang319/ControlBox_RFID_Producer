#ifndef OFFLINEDATAREPORTTHREAD_H
#define OFFLINEDATAREPORTTHREAD_H

#include <thread>
#include <vector>

class OfflineDataReportThread
{
public:
    OfflineDataReportThread();

    ~OfflineDataReportThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;

    void threadFunction();
};

#endif // OFFLINEDATAREPORTTHREAD_H
