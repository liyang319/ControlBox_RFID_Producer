#include "SensorWorkerThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "MsgDispatcher.h"
#include "Base.h"
#include "Dwin.h"
#include "SensorData.h"
#include "WeighData.h"
#include "SensorMsgDispatcher.h"

using namespace std;
using namespace rapidjson;

SensorWorkerThread::SensorWorkerThread() : m_running(false)
{
}

SensorWorkerThread::~SensorWorkerThread()
{
    stop();
}

void SensorWorkerThread::init()
{
    // WeighData::getInstance().init();
}

void SensorWorkerThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&SensorWorkerThread::threadFunction, this);
    }
}

void SensorWorkerThread::stop()
{
    if (m_running)
    {
        m_running = false;
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }
}

void SensorWorkerThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void SensorWorkerThread::threadFunction()
{
    while (m_running)
    {
        if (SensorData::getInstance().getDataRecvQueueSize() > 0)
        {
            SensorComUnit dataUnit = SensorData::getInstance().getDataFromDataRecvQueue();
            SensorMsgDispatcher dispatcher;
            dispatcher.dispatchMsg(dataUnit);
        }
        usleep(10000);
    }
}
