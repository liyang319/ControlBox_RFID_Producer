#ifndef GPSCOMTHREAD_H
#define GPSCOMTHREAD_H

#include <thread>
#include <vector>

class GpsComThread
{
public:
    GpsComThread();

    ~GpsComThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;
    void init();

    void threadFunction();
};

#endif // GPSCOMTHREAD_H
