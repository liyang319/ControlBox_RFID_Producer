#ifndef MSGWORKERTHREAD_H
#define MSGWORKERTHREAD_H

#include <thread>
#include <vector>

class MsgWorkerThread
{
public:
    MsgWorkerThread();

    ~MsgWorkerThread();

    void start();

    void stop();

    void join(); // 添加join方法

private:
    std::thread m_thread;
    bool m_running;

    void threadFunction();
    std::vector<std::string> splitJsonCmd(std::string strCmd);
};

#endif // MSGWORKERTHREAD_H
