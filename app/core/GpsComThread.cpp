#include "GpsComThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "MsgDispatcher.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Base.h"
#include "GPS.h"
#include "WeighData.h"
#include "Utility.h"
#include "GlobalFlag.h"

using namespace std;
using namespace rapidjson;

GpsComThread::GpsComThread() : m_running(false)
{
}

GpsComThread::~GpsComThread()
{
    stop();
}

void GpsComThread::init()
{
    sleep(10);
}

void GpsComThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&GpsComThread::threadFunction, this);
    }
}

void GpsComThread::stop()
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

void GpsComThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void GpsComThread::threadFunction()
{
    while (m_running)
    {
        if (GlobalFlag::getInstance().isUpdating())
        {
            usleep(1000000);
            continue;
        }
        // std::cout << "-------GpsComThread-------" << endl;
        GPS::getInstance().readData(WeighData::getInstance().currentWeighData.locationData);
        sleep(1);
    }
}
