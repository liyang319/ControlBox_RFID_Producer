#ifndef EXCEPTIONDATAPROCESSTHREAD_H
#define EXCEPTIONDATAPROCESSTHREAD_H

#include <thread>
#include <vector>

class ExceptionDataProcessThread
{
public:
    ExceptionDataProcessThread();

    ~ExceptionDataProcessThread();

    void start();

    void stop();

    void join(); // 添加join方法

    void init();

private:
    std::thread m_thread;
    bool m_running;

    void threadFunction();
};

#endif // EXCEPTIONDATAPROCESSTHREAD_H
