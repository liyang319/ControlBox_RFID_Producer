#include "DataReportThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "WeighData.h"
#include "Base.h"
#include "DataDef.h"
#include "GlobalFlag.h"

using namespace std;

DataReportThread::DataReportThread() : m_running(false), loopIndex(0), publishInterval(2)
{
    // init();
}

DataReportThread::~DataReportThread()
{
    stop();
}

void DataReportThread::init()
{
    publishInterval = GlobalFlag::getInstance().publishinterval;
}

void DataReportThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&DataReportThread::threadFunction, this);
    }
}

void DataReportThread::stop()
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

void DataReportThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void DataReportThread::threadFunction()
{
    while (m_running)
    {
        if (GlobalFlag::getInstance().isUpdating())
        {
            usleep(1000000);
            continue;
        }
        if (loopIndex % publishInterval == 0)
        {
            // cout << "-----DataReportThread-------" << std::dec << loopIndex << endl;
            string reportData = WeighData::getInstance().getReportData();
            if (reportData != "")
            {
                MqttPublishUnit reportUnit;
                reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_DATA;
                reportUnit.content = reportData;
                AppData::getInstance().addDataToDataSendQueue(reportUnit);
            }
        }
        loopIndex++;
        usleep(1000000);
    }
}
