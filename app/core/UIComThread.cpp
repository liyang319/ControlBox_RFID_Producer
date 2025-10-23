#include "UIComThread.h"
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
#include "Dwin.h"
#include "UIMsgDispatcher.h"
#include "GC31.h"
#include "WeighData.h"
#include "Utility.h"
#include "GlobalFlag.h"
#include "Version.h"

using namespace std;
using namespace rapidjson;

UIComThread::UIComThread() : m_running(false)
{
}

UIComThread::~UIComThread()
{
    stop();
}

void UIComThread::init()
{
    // std::cout << "---------------UIComThread---------------" << std::endl;
    // GC31::getInstance().init();
    // Dwin::getInstance().init();
    uiWorkerThread.start();
    Dwin::getInstance().syncSystemTime();
    Dwin::getInstance().switchPage(UI_DWIN_PAGE_MAIN);
    string strVer = VERSION;
    float initVal = 0;
    string strSN = "SN: " + Utility::getDeviceSN();
    Dwin::getInstance().writeFloatValue(UI_ADDR_NET_WEIGHT, initVal);
    Dwin::getInstance().writeFloatValue(UI_ADDR_GROSS_WEIGHT, initVal);
    Dwin::getInstance().writeFloatValue(UI_ADDR_TARE_WEIGHT, initVal);
    Dwin::getInstance().writeStringValue(UI_ADDR_DEVICE_SN, strSN);
    Dwin::getInstance().setIcon(UI_ADDR_LOCAL_MODE_SWITCH, GlobalFlag::getInstance().bEdgeComputeMode ? UI_KEY_SWITCH_ON : UI_KEY_SWITCH_OFF);
    Dwin::getInstance().setIcon(UI_ADDR_OFFLINE_MODE_SWITCH, GlobalFlag::getInstance().bOfflineMode ? UI_KEY_SWITCH_ON : UI_KEY_SWITCH_OFF);
    Dwin::getInstance().writeFloatValue(UI_ADDR_TARE_WEIGHT, WeighData::getInstance().currentTareWeight);
    Dwin::getInstance().setIcon(UI_ADDR_OVERLOAD_ICON, UI_KEY_OVERLOAD_ALARM_OFF);
    Dwin::getInstance().writeStringValue(UI_ADDR_MAIN_VERSION, strVer);
}

void UIComThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&UIComThread::threadFunction, this);
    }
}

void UIComThread::stop()
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

void UIComThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void UIComThread::threadFunction()
{
    uint8_t dataBuf[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
    UIMsgDispatcher dispatcher;
    while (m_running)
    {
        /////////////////////////////升级时不处理键盘事件/////////////////////////////////////
        if (GlobalFlag::getInstance().isUpdating())
        {
            usleep(1000000);
            continue;
        }
        ///////////////////////////////////键盘事件处理/////////////////////////////////////
        int rxLen = Dwin::getInstance().readData(dataBuf, sizeof(dataBuf));
        if (rxLen > 0)
        {
            dispatcher.dispatchMsg(dataBuf, rxLen);
        }
        ///////////////////////////////////UI数据处理/////////////////////////////////////
        if (UIData::getInstance().getDataSendQueueSize() > 0)
        {
            UIDataUnit uiData = UIData::getInstance().getDataFromDataSendQueue();
            if (uiData.cmd == UI_CMD_UPDATE_WEIGHT)
            {
                UpdateWeight(uiData);
            }
            else if (uiData.cmd == UI_CMD_OVERLOAD_ALARM_ON || uiData.cmd == UI_CMD_OVERLOAD_ALARM_OFF)
            {
                UpdateOverloadAlarm(uiData);
            }
            // else if (uiData.cmd == UI_CMD_UPDATE_DWIN)
            // {
            //     UpdateDwin(uiData);
            // }
            // else if (uiData.cmd == UI_CMD_SWITCH_PAGE)
            // {
            //     SwitchPage(uiData);
            // }
        }
        sleep(1);
    }
}

void UIComThread::UpdateWeight(UIDataUnit data)
{
    cout << "------UIComThread::UpdateWeight-----" << endl;
    float netWeight;
    memcpy(&netWeight, data.content, sizeof(float));
    printf("------netWeight-------%f\n", netWeight);
    if (data.content != nullptr)
    {
        delete[] data.content;
        data.content = nullptr;
    }
    if (netWeight < 0)
        netWeight = 0;
    float grossWeight = netWeight + WeighData::getInstance().currentTareWeight;
    Dwin::getInstance().writeFloatValue(UI_ADDR_NET_WEIGHT, ((netWeight < 0) ? 0 : netWeight));
    // usleep(10000);
    // Dwin::getInstance().writeFloatValue(UI_ADDR_TARE_WEIGHT, WeighData::getInstance().currentTareWeight);
    Dwin::getInstance().writeFloatValue(UI_ADDR_GROSS_WEIGHT, ((grossWeight < 0) ? 0 : grossWeight));
    // usleep(10000);
}

void UIComThread::UpdateOverloadAlarm(UIDataUnit data)
{
    cout << "------UIComThread::UpdateOverloadAlarm-----" << endl;
    if (data.cmd == UI_CMD_OVERLOAD_ALARM_ON)
    {
        Dwin::getInstance().setIcon(UI_ADDR_OVERLOAD_ICON, UI_KEY_OVERLOAD_ALARM_ON);
    }
    else
    {
        Dwin::getInstance().setIcon(UI_ADDR_OVERLOAD_ICON, UI_KEY_OVERLOAD_ALARM_OFF);
    }
    if (data.content != nullptr)
    {
        delete[] data.content;
        data.content = nullptr;
    }
}

void UIComThread::UpdateDwin(UIDataUnit data)
{
    cout << "------UIComThread::UpdateDwin-----" << endl;
    string pkgName = string(reinterpret_cast<const char *>(data.content), data.len);

    if (data.content != nullptr)
    {
        delete[] data.content;
        data.content = nullptr;
    }
}

void UIComThread::SwitchPage(UIDataUnit data)
{
    cout << "------UIComThread::SwitchPage-----" << endl;
    int page = 0;
    memcpy(&page, data.content, sizeof(int));

    if (data.content != nullptr)
    {
        delete[] data.content;
        data.content = nullptr;
    }
}
