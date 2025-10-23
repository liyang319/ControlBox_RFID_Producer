#ifndef UICOMTHREAD_H
#define UICOMTHREAD_H

#include <thread>
#include <vector>
#include "UIWorkerThread.h"

class UIComThread
{
public:
    UIComThread();

    ~UIComThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();

    UIWorkerThread uiWorkerThread;
    void threadFunction();

    void UpdateWeight(UIDataUnit data);
    void UpdateOverloadAlarm(UIDataUnit data);
    void UpdateDwin(UIDataUnit data);
    void SwitchPage(UIDataUnit data);
};

#endif // UICOMTHREAD_H
