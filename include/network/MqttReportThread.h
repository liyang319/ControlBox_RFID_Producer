#ifndef MQTTREPORTERTHREAD_H
#define MQTTREPORTERTHREAD_H

#include <thread>
#include <vector>

class MqttThread;

class MqttReportThread
{
public:
    MqttReportThread(MqttThread *mqttThrea);

    ~MqttReportThread();

    void start();

    void stop();

    void join();

private:
    std::thread m_thread;
    bool m_running;
    MqttThread *m_mqttThread;
    void threadFunction();
};

#endif // MQTTREPORTERTHREAD_H
