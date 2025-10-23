#include "MqttReportThread.h"
#include "MqttThread.h"
#include "AppData.h"
#include "Base.h"
#include <unistd.h>

MqttReportThread::MqttReportThread(MqttThread *mqttThread) : m_mqttThread(mqttThread), m_running(false) {}

MqttReportThread::~MqttReportThread()
{
    stop();
}

void MqttReportThread::start()
{
    m_thread = std::thread(&MqttReportThread::threadFunction, this);
}

void MqttReportThread::stop()
{
    m_running = false;
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void MqttReportThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void MqttReportThread::threadFunction()
{
    m_running = true;
    int index = 0;
    while (m_running)
    {
        if (m_mqttThread->isMqttConnected())
        {
            if (AppData::getInstance().getDataSendQueueSize() > 0)
            {
                MqttPublishUnit sendData = AppData::getInstance().getDataFromDataSendQueue();
                m_mqttThread->publish(sendData, 0, false);
            }
        }
        // std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒发布一次数据
        usleep(200000);
    }
}
