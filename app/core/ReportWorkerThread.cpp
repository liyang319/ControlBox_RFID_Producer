#include "ReportWorkerThread.h"
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
#include "DeviceConfig.h"
#include "GC31.h"
#include "TDA04d.h"
#include "GPS.h"
#include "ZY4701.h"
#include "WeighAlgorithm.h"
#include "Dwin.h"
#include "UIData.h"
#include "DataDef.h"
#include "MdtuProv.h"
#include "Utility.h"
#include <sstream>
#include "DBWeighData.h"
#include "GlobalFlag.h"
#include "CanData.h"

using namespace std;
using namespace rapidjson;

ReportWorkerThread::ReportWorkerThread() : m_running(false)
{
    handleflag.store(0);
    loopIndex = 1;
}

ReportWorkerThread::~ReportWorkerThread()
{
    stop();
}

void ReportWorkerThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&ReportWorkerThread::threadFunction, this);
    }
}

void ReportWorkerThread::stop()
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

void ReportWorkerThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void ReportWorkerThread::threadFunction()
{
    sleep(10); // 等待前面初始化的消息都处理完成
    // 初始化GC31时间表
    if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
    {
        GC31::getInstance().initTimer(WeighData::getInstance().currentWeighData.sensorDataMap);
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
    {
        TDA04d::getInstance().initTimer(WeighData::getInstance().currentWeighData.sensorDataMap);
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
    {
        ZY4701::getInstance().initTimer(WeighData::getInstance().currentWeighData.sensorDataMap);
    }

    while (m_running)
    {
        if (GlobalFlag::getInstance().isUpdating())
        {
            usleep(1000000);
            continue;
        }
        handleflag.fetch_add(1);
        getDeviceData();
        // getLocationData();
        WeighData::getInstance().currentWeighData.handleflag = handleflag;
        // WeighData::getInstance().weigh_data_record_queue.addDataToQueue(WeighData::getInstance().currentWeighData);
        string reportData = "";

        if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
        {
            AlgorithmResult caculateResult;
            if (WeighAlgorithm::getInstance().LowPassFiltering(WeighData::getInstance().currentWeighData.locationData.sv_num,
                                                               WeighData::getInstance().currentWeighData.locationData.gps_v,
                                                               WeighData::getInstance().currentWeighData.sensorDataMap[DEFAULT_TDA04D_ADDR].chVal,
                                                               2, caculateResult))
            {
                if (GlobalFlag::getInstance().bCompresReport)
                {
                    reportData = DataFormater::FormatSensorLocationGroupData_FullData_Compress(WeighData::getInstance().currentWeighData, caculateResult);
                }
                else
                {
                    reportData = DataFormater::FormatSensorLocationGroupData_FullData(WeighData::getInstance().currentWeighData, caculateResult);
                }
            }

            // UI显示结果
            float result = static_cast<float>(caculateResult.wcal) / 10;
            UIDataUnit uiData;
            uiData.cmd = UI_CMD_UPDATE_WEIGHT;
            uiData.content = new unsigned char[sizeof(float)];
            memcpy(uiData.content, (uint8_t *)&result, sizeof(float));
            UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数

            if (result > WeighData::getInstance().loadCap)
            {
                if (!WeighData::getInstance().bOverload)
                {
                    UIDataUnit uiData;
                    uiData.cmd = UI_CMD_OVERLOAD_ALARM_ON;
                    UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                    WeighData::getInstance().bOverload = true;
                }
            }
            else
            {
                if (WeighData::getInstance().bOverload)
                {
                    UIDataUnit uiData;
                    uiData.cmd = UI_CMD_OVERLOAD_ALARM_OFF;
                    UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                    WeighData::getInstance().bOverload = false;
                }
            }

            if (GlobalFlag::getInstance().bOfflineMode || (!GlobalFlag::getInstance().bDataCanReport)) // 离线模式，保存至数据库
            {
                COUT << "-----------offline----saveData-----handleflag=" << std::dec << handleflag.load() << endl;
                DBWeighData::getInstance().inserWeightData(handleflag.load(), Utility::getTimestamp(), reportData);
                reportData = ""; // 保存完数据库后，清除变量，这样DataReportThread中不会上报
            }
            else // 非离线模式，上传mqtt
            {
                COUT << "---------------report-------handleflag=" << std::dec << handleflag.load() << endl;
                // MqttPublishUnit reportUnit;
                // reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_DATA;
                // reportUnit.content = reportData;
                // AppData::getInstance().addDataToDataSendQueue(reportUnit);
            }
        }
        else if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31 || GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
        {
            if (GlobalFlag::getInstance().bEdgeComputeMode || GlobalFlag::getInstance().bOfflineMode || (!GlobalFlag::getInstance().bDataCanReport))
            {
                WeighData::getInstance().weigh_data_record_queue.addDataToQueue(WeighData::getInstance().currentWeighData);
                AlgorithmResult caculateResult;
                if (WeighAlgorithm::getInstance().GetAlgorithmResult(handleflag, caculateResult))
                {
                    WeighData::getInstance().weigh_data_record_queue.removeDataFromQueue(GlobalFlag::getInstance().winstep);
                }

                if (GlobalFlag::getInstance().bReportFullData)
                {
                    if (GlobalFlag::getInstance().bCompresReport)
                    {
                        reportData = DataFormater::FormatSensorLocationGroupData_FullData_Compress(WeighData::getInstance().currentWeighData, caculateResult);
                    }
                    else
                    {
                        reportData = DataFormater::FormatSensorLocationGroupData_FullData(WeighData::getInstance().currentWeighData, caculateResult);
                    }
                }
                else
                {
                    if (GlobalFlag::getInstance().bCompresReport)
                    {
                        reportData = DataFormater::FormatSensorLocationGroupData_LocalMode_Compress(WeighData::getInstance().currentWeighData, caculateResult);
                    }
                    else
                    {
                        reportData = DataFormater::FormatSensorLocationGroupData_LocalMode(WeighData::getInstance().currentWeighData, caculateResult);
                    }
                }
                // UI显示结果
                float result = static_cast<float>(caculateResult.wcal) / 1000;
                printf("---ReportWorkerThread---result-------%f\n", result);
                UIDataUnit uiData;
                uiData.cmd = UI_CMD_UPDATE_WEIGHT;
                uiData.content = new unsigned char[sizeof(float)];
                memcpy(uiData.content, (uint8_t *)&result, sizeof(float));
                UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数

                // Can Data
                if (GlobalFlag::getInstance().bCan)
                {
                    CanDataUnit canData;
                    canData.cmd = CAN_CMD_UPDATE_WEIGHT;
                    canData.content = new unsigned char[sizeof(float)];
                    memcpy(canData.content, (uint8_t *)&result, sizeof(float));
                    CanData::getInstance().addDataToDataSendQueue(canData); // 更新UI数
                }

                if (result > WeighData::getInstance().loadCap)
                {
                    if (!WeighData::getInstance().bOverload)
                    {
                        UIDataUnit uiData;
                        uiData.cmd = UI_CMD_OVERLOAD_ALARM_ON;
                        UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                        WeighData::getInstance().bOverload = true;
                    }
                }
                else
                {
                    if (WeighData::getInstance().bOverload)
                    {
                        UIDataUnit uiData;
                        uiData.cmd = UI_CMD_OVERLOAD_ALARM_OFF;
                        UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                        WeighData::getInstance().bOverload = false;
                    }
                }

                if (GlobalFlag::getInstance().bOfflineMode || (!GlobalFlag::getInstance().bDataCanReport)) // 离线模式，保存至数据库
                {
                    COUT << "-----------offline----saveData-----handleflag=" << std::dec << handleflag.load() << endl;
                    DBWeighData::getInstance().inserWeightData(handleflag.load(), Utility::getTimestamp(), reportData);
                    reportData = ""; // 保存完数据库后，清除变量，这样DataReportThread中不会上报
                }
                else // 非离线模式，上传mqtt
                {
                    COUT << "---------------report-------handleflag=" << std::dec << handleflag.load() << endl;
                    // MqttPublishUnit reportUnit;
                    // reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_DATA;
                    // reportUnit.content = reportData;
                    // AppData::getInstance().addDataToDataSendQueue(reportUnit);
                }
            }
            else
            {
                COUT << "-----------cloud-----report-------handleflag=" << std::dec << handleflag.load() << endl;
                AlgorithmResult caculateResult = {};
                if (GlobalFlag::getInstance().bCompresReport)
                {
                    reportData = DataFormater::FormatSensorLocationGroupData_FullData_Compress(WeighData::getInstance().currentWeighData, caculateResult);
                }
                else
                {
                    reportData = DataFormater::FormatSensorLocationGroupData_FullData(WeighData::getInstance().currentWeighData, caculateResult);
                }
                // MqttPublishUnit reportUnit;
                // reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_DATA;
                // reportUnit.content = reportData;
                // AppData::getInstance().addDataToDataSendQueue(reportUnit);
            }
        }
        // else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
        // {
        // }
        WeighData::getInstance().saveReportData(reportData);
        cout << "---" << reportData << endl;

        // sleep(2);
        usleep(1500000);
    }
}

void ReportWorkerThread::init()
{
    // GC31::getInstance().init();
    // GPS::getInstance().init();
    // WeighData::getInstance().init();
    // 打开数据库连接
    // if (!weighDataDB.open())
    // {
    //     std::cout << "Failed to open database." << std::endl;
    //     return;
    // }
    // std::string columns = "id INTEGER PRIMARY KEY, timestamp INTEGER, weigh_info TEXT";
    // if (weighDataDB.createTable(DEFAULT_DB_TABLE_NAME, columns))
    // {
    //     std::cout << "Table weigh_data created successfully." << std::endl;
    // }
    // else
    // {
    //     std::cout << "Failed to create table weigh_data." << std::endl;
    // }

    usleep(100000);
}

void ReportWorkerThread::getLocationData()
{
    COUT << "--------getLocationData-------" << endl;
    if (!GPS::getInstance().isConnected())
    {
        COUT << "GPS serial not connected!!!" << endl;
        return;
    }
    GPS::getInstance().readData(WeighData::getInstance().currentWeighData.locationData);
    // string reportData = DataFormater::FormatLocationData(WeighData::getInstance().currentWeighData.locationData, handleflag);
    // AppData::getInstance().addDataToDataSendQueue(reportData);
}

void ReportWorkerThread::getDeviceData()
{
    COUT << "--------getDeviceData-------" << endl;
    if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
    {
        if (!GC31::getInstance().isConnected())
        {
            COUT << "GC31 serial not connected!!!" << endl;
            return;
        }
        for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
        {
            SensorDataUnit *pData = &(it->second);
            if (pData->bActive)
            {
                GC31::getInstance().readMeasureData(pData->sensorAddr);
                if (pData->bSettingUpdated)
                {
                    GC31::getInstance().getDeviceSetting(pData->sensorAddr);
                }

                // string reportData = DataFormater::FormatSensorData(*pData, handleflag);
                // AppData::getInstance().addDataToDataSendQueue(reportData); // mqtt上报
                usleep(100000);
            }
        }
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
    {
        if (!TDA04d::getInstance().isConnected())
        {
            COUT << "TDA04d serial not connected!!!" << endl;
            return;
        }
        for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
        {
            SensorDataUnit *pData = &(it->second);
            if (pData->bActive)
            {
                TDA04d::getInstance().readMeasureData(pData->sensorAddr);
                usleep(100000);
            }
        }
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
    {
        if (!ZY4701::getInstance().isConnected())
        {
            COUT << "ZY4701 serial not connected!!!" << endl;
            return;
        }
        for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
        {
            SensorDataUnit *pData = &(it->second);
            if (pData->bActive)
            {
                bool bSuccess = false;
                COUT << "--------worker------bSettingUpdated----" << pData->bSettingUpdated << endl;
                if (pData->bSettingUpdated)
                {
                    for (int i = 0; i < SENSOR_MAX_NUM; i++)
                    {
                        bSuccess = ZY4701::getInstance().getDeviceSetting(pData->sensorAddr, i, pData->settings.gain[i], pData->settings.reverseFlag[i], pData->settings.filter[i], pData->settings.autoCalib[i]);
                    }
                    if (bSuccess)
                    {
                        pData->bSettingUpdated = false;
                    }
                }
                ZY4701::getInstance().readMeasureData(pData->sensorAddr, pData->tempVal[0], pData->chVal);
                usleep(100000);
            }
        }
    }
}