#ifndef UIWORKERTHREAD_H
#define UIWORKERTHREAD_H

#include <thread>
#include <vector>
#include "UIData.h"

class UIWorkerThread
{
public:
    UIWorkerThread();

    ~UIWorkerThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();

    void threadFunction();

    void processZeroCalibCmd();
    void processResetDeviceCmd();
    void processExceptionCmd();
    void processSetTareCmd();
    void processExceptionPreviousCmd();
    void processExceptionNextCmd();

    void processSetLocalMode(UIDataUnit dataUnit);
    void processSetOfflineMode(UIDataUnit dataUnit);
    void showExceptionContent(int pageIndex);
    void clearExceptionContent();
};

#endif // UIWORKERTHREAD_H
