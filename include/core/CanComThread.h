#ifndef CANCOMTHREAD_H
#define CANCOMTHREAD_H

#include <thread>
#include <vector>
#include "CanData.h"

class CanComThread
{
public:
    CanComThread();

    ~CanComThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();

    void threadFunction();

    void UpdateWeight(CanDataUnit data);
};

#endif // CANCOMTHREAD_H
