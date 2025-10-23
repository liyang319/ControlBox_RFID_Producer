#include "UIWorkerThread.h"
#include "AppData.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include "AppData.h"
#include "MsgDispatcher.h"
#include "Base.h"
#include "Dwin.h"
#include "UIData.h"
#include "WeighData.h"
#include "Utility.h"
#include "DataFormater.h"
#include "DataDef.h"
#include "GlobalFlag.h"
#include "DBExceptionData.h"

using namespace std;
using namespace rapidjson;

uint16_t UI_ADDR_EXCEPTION_MAP[] = {UI_ADDR_EXCEPTION_CONTENT_1, UI_ADDR_EXCEPTION_CONTENT_2, UI_ADDR_EXCEPTION_CONTENT_3};

UIWorkerThread::UIWorkerThread() : m_running(false)
{
}

UIWorkerThread::~UIWorkerThread()
{
    stop();
}

void UIWorkerThread::init()
{
    // Dwin::getInstance().init();
    // WeighData::getInstance().init();
}

void UIWorkerThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&UIWorkerThread::threadFunction, this);
    }
}

void UIWorkerThread::stop()
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

void UIWorkerThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void UIWorkerThread::threadFunction()
{
    while (m_running)
    {
        if (UIData::getInstance().getDataRecvQueueSize() > 0)
        {
            UIDataUnit dataUnit = UIData::getInstance().getDataFromDataRecvQueue();
            if (dataUnit.cmd == UI_MSG_CMD_ZEROCALIB)
            {
                processZeroCalibCmd();
            }
            else if (dataUnit.cmd == UI_MSG_CMD_SET_TARE)
            {
                processSetTareCmd();
            }
            if (dataUnit.cmd == UI_MSG_CMD_RESET_DEVICE)
            {
                processResetDeviceCmd();
            }
            else if (dataUnit.cmd == UI_MSG_CMD_EXCETION_GET)
            {
                processExceptionCmd();
            }
            else if (dataUnit.cmd == UI_MSG_CMD_EXCETION_PREVIOUS)
            {
                processExceptionPreviousCmd();
            }
            else if (dataUnit.cmd == UI_MSG_CMD_EXCETION_NEXT)
            {
                processExceptionNextCmd();
            }
            else if (dataUnit.cmd == UI_MSG_CMD_SET_LOCAL_MODE)
            {
                processSetLocalMode(dataUnit);
            }
            else if (dataUnit.cmd == UI_MSG_CMD_SET_OFFLINE_MODE)
            {
                processSetOfflineMode(dataUnit);
            }
            if (dataUnit.content != nullptr) // delete new出来的空间
            {
                delete[] dataUnit.content;
                dataUnit.content = nullptr;
            }
        }
        sleep(1);
    }
}

void UIWorkerThread::processZeroCalibCmd()
{
    cout << "--------UIWorkerThread::processZeroCalibCmd-------" << endl;
    for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
    {
        SensorDataUnit *pData = &(it->second);
        if (pData->bActive)
        {
            GC31::getInstance().setEmptyLoadValue(pData->sensorAddr, DEFAULT_ZEROCALIB_VALUE);
        }
    }
}

void UIWorkerThread::processResetDeviceCmd()
{
    cout << "--------UIWorkerThread::processResetDeviceCmd-------" << endl;
    // for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
    // {
    //     SensorDataUnit *pData = &(it->second);
    //     if (pData->bActive)
    //     {
    //         GC31::getInstance().resetDevice(pData->sensorAddr);
    //     }
    // }
    Utility::rebootSystem();
    return;
}

void UIWorkerThread::processExceptionCmd()
{
    cout << "--------UIWorkerThread::processExceptionCmd-------" << endl;
    // string str = "123123123213你好啊大地方\r\n阿斯顿发送到发大水发达";
    // Dwin::getInstance().writeStringValue(UI_ADDR_EXCEPTION_CONTENT_1, str);
    DBExceptionData::getInstance().loadExceptionsFromDB();
    DBExceptionData::getInstance().getPageNum();
    showExceptionContent(DBExceptionData::getInstance().currentExpPageIndex);
    return;
}

void UIWorkerThread::processSetTareCmd()
{
    cout << "--------UIWorkerThread::processSetTareCmd-------" << endl;
    float tareInput;
    Dwin::getInstance().readFloatValue(UI_ADDR_TARE_INPUT, tareInput);
    WeighData::getInstance().currentTareWeight = tareInput;
    Dwin::getInstance().writeFloatValue(UI_ADDR_TARE_WEIGHT, WeighData::getInstance().currentTareWeight);
    // Utility::saveTareWeight(DEFAULT_WEIGHT_INFO_PATH, WeighData::getInstance().currentTareWeight);
    Utility::writeMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_TAREWEIGHT, std::to_string(WeighData::getInstance().currentTareWeight));
    return;
}

void UIWorkerThread::processExceptionPreviousCmd()
{
    cout << "--------UIWorkerThread::processExceptionPreviousCmd-------" << endl;
    if (DBExceptionData::getInstance().currentExpPageIndex > 0)
    {
        DBExceptionData::getInstance().currentExpPageIndex--;
        showExceptionContent(DBExceptionData::getInstance().currentExpPageIndex);
    }
    else
    {
        DBExceptionData::getInstance().currentExpPageIndex = 0;
    }
    return;
}

void UIWorkerThread::processExceptionNextCmd()
{
    cout << "--------UIWorkerThread::processExceptionNextCmd-------" << endl;
    int pageNum = DBExceptionData::getInstance().getPageNum();
    if (DBExceptionData::getInstance().currentExpPageIndex >= pageNum - 1)
    {
        DBExceptionData::getInstance().currentExpPageIndex = pageNum - 1;
    }
    else
    {
        DBExceptionData::getInstance().currentExpPageIndex++;
        showExceptionContent(DBExceptionData::getInstance().currentExpPageIndex);
    }
    return;
}

void UIWorkerThread::processSetLocalMode(UIDataUnit dataUnit)
{
    cout << "--------UIWorkerThread::processSetLocalMode-------" << endl;
    uint16_t keyValue = (dataUnit.content[0] << 8) | dataUnit.content[1];
    std::cout << "keyValue: 0x" << std::hex << std::setw(4) << std::setfill('0') << keyValue << std::endl;
    GlobalFlag::getInstance().bEdgeComputeMode = keyValue > 0 ? true : false;
    Dwin::getInstance().setIcon(UI_ADDR_MAIN_ICON1, keyValue > 0 ? UI_ICON_INDEX_EDGE_MODE_ON : UI_ICON_INDEX_CLOUD_MODE_ON);
    // 上报计算模式
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    int localMode = COMPUTE_MODE_CLOUD;
    if (GlobalFlag::getInstance().bEdgeComputeMode && GlobalFlag::getInstance().bReportFullData)
    {
        localMode = COMPUTE_MODE_EDGE_ALL;
    }
    else if (GlobalFlag::getInstance().bEdgeComputeMode)
    {
        localMode = COMPUTE_MODE_EDGE_SIMPLE;
    }
    reportUnit.content = DataFormater::FormatModeData(localMode, GlobalFlag::getInstance().bOfflineMode);
    AppData::getInstance().addDataToDataSendQueue(reportUnit);
    Utility::writeMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_EDGE, to_string(localMode)); // 保存在自定义配置文件中
    if (GlobalFlag::getInstance().bEdgeComputeMode)
    {
        cout << "-----clear-----weigh_data_record_queue----" << endl;
        WeighData::getInstance().weigh_data_record_queue.clear();
    }
    return;
}

void UIWorkerThread::processSetOfflineMode(UIDataUnit dataUnit)
{
    cout << "--------UIWorkerThread::processSetOfflineMode-------" << endl;
    uint16_t keyValue = (dataUnit.content[0] << 8) | dataUnit.content[1];
    std::cout << "keyValue: 0x" << std::hex << std::setw(4) << std::setfill('0') << keyValue << std::endl;
    GlobalFlag::getInstance().bOfflineMode = keyValue > 0 ? true : false;
    Dwin::getInstance().setIcon(UI_ADDR_MAIN_ICON2, keyValue > 0 ? UI_ICON_INDEX_OFFLINE_MODE_ON : UI_ICON_INDEX_OFF);
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    // reportUnit.content = DataFormater::FormatModeData(GlobalFlag::getInstance().bEdgeComputeMode, GlobalFlag::getInstance().bOfflineMode);
    int localMode = COMPUTE_MODE_CLOUD;
    if (GlobalFlag::getInstance().bEdgeComputeMode && GlobalFlag::getInstance().bReportFullData)
    {
        localMode = COMPUTE_MODE_EDGE_ALL;
    }
    else if (GlobalFlag::getInstance().bEdgeComputeMode)
    {
        localMode = COMPUTE_MODE_EDGE_SIMPLE;
    }
    reportUnit.content = DataFormater::FormatModeData(localMode, GlobalFlag::getInstance().bOfflineMode);
    AppData::getInstance().addDataToDataSendQueue(reportUnit);
    Utility::writeMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_OFFLINE, GlobalFlag::getInstance().bOfflineMode ? "1" : "0"); // 保存在自定义配置文件中
    return;
}

void UIWorkerThread::showExceptionContent(int pageIndex)
{
    clearExceptionContent();
    int pageNum = DBExceptionData::getInstance().getPageNum();
    if (pageIndex > pageNum)
    {
        pageIndex = pageNum;
    }
    if (pageIndex < 0)
    {
        pageIndex = 0;
    }
    std::vector<ExceptionDBDataUnit> myVec;
    myVec.clear();
    DBExceptionData::getInstance().getExceptions(pageIndex, myVec, DEFAULT_EXCEPTIONS_UI_PAGE_SIZE);
    for (size_t i = 0; i < myVec.size(); ++i)
    {
        const ExceptionDBDataUnit &exception = myVec[i];
        std::cout << "Index: " << i
                  << ", Timestamp: " << exception.timestamp
                  << ", Exception Code: " << static_cast<int>(exception.exceptionCode)
                  << ", Exp Sensors: " << static_cast<int>(exception.expSensors)
                  << ", Exp GC31s: " << static_cast<int>(exception.expGC31s) << std::endl;
        string expStr = DataFormater::FormatExceptionUIContent(exception);
        Dwin::getInstance().writeStringValue(UI_ADDR_EXCEPTION_MAP[i], expStr);
    }
}

void UIWorkerThread::clearExceptionContent()
{
    for (int i = 0; i < DEFAULT_EXCEPTIONS_UI_PAGE_SIZE; i++)
    {
        Dwin::getInstance().clearStringValue(UI_ADDR_EXCEPTION_MAP[i], DEFAULT_EXCEPTIONS_UI_CONTENT_SIZE);
    }
}
