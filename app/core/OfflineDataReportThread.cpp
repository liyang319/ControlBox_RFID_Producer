#include "OfflineDataReportThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "DBWeighData.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Base.h"
#include "WeighData.h"
#include "GlobalFlag.h"

using namespace std;
using namespace rapidjson;

OfflineDataReportThread::OfflineDataReportThread() : m_running(false)
{
}

OfflineDataReportThread::~OfflineDataReportThread()
{
    stop();
}

void OfflineDataReportThread::start()
{
    if (!m_running)
    {
        m_running = true;
        m_thread = std::thread(&OfflineDataReportThread::threadFunction, this);
    }
}

void OfflineDataReportThread::stop()
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

void OfflineDataReportThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void OfflineDataReportThread::threadFunction()
{
    sleep(10);
    COUT << "---begin----" << endl;
    while (m_running)
    {
        if (!GlobalFlag::getInstance().bOfflineMode && GlobalFlag::getInstance().bDataCanReport && !DBWeighData::getInstance().isDbEmpty(DEFAULT_DB_TABLE_NAME))
        {
            std::vector<OfflineDBDataUnit> dataList;
            DBWeighData::getInstance().processAndDeleteData(5, dataList);
            for (const auto &dataUnit : dataList)
            {
                COUT << "Timestamp: " << dataUnit.timestamp << ", Handleflag: " << dataUnit.handleflag << ", WeighInfo: " << dataUnit.weighInfo << std::endl;
                MqttPublishUnit reportUnit = {};
                reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_OFFLINEDATA;
                reportUnit.content = dataUnit.weighInfo;
                AppData::getInstance().addDataToDataSendQueue(reportUnit);
            }
        }
        sleep(5);
    }
}
