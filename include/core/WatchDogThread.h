#ifndef WATCHDOGTHREAD_H
#define WATCHDOGTHREAD_H

#include <thread>
#include <vector>

#define DEFAULT_WATCHDOG_FEED_INTERVAL 5

class WatchDogThread
{
public:
    WatchDogThread();

    ~WatchDogThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;

    void threadFunction();
};

#endif // WATCHDOGTHREAD_H
