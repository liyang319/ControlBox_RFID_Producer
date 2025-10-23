#include "CanComThread.h"
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
#include "WeighData.h"
#include "Utility.h"
#include "GlobalFlag.h"
#include "Version.h"
#include "CanData.h"
#include "CanManager.h"

using namespace std;
using namespace rapidjson;

CanComThread::CanComThread() : m_running(false)
{
}

CanComThread::~CanComThread()
{
    stop();
}

void CanComThread::init()
{
    // std::cout << "---------------UIComThread---------------" << std::endl;
    // uiWorkerThread.start();
}

void CanComThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&CanComThread::threadFunction, this);
    }
}

void CanComThread::stop()
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

void CanComThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void CanComThread::threadFunction()
{
    uint8_t dataBuf[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
    while (m_running)
    {
        /////////////////////////////升级时不处理事件/////////////////////////////////////
        if (GlobalFlag::getInstance().isUpdating())
        {
            usleep(1000000);
            continue;
        }
        ///////////////////////////////////UI数据处理/////////////////////////////////////
        if (CanData::getInstance().getDataSendQueueSize() > 0)
        {
            CanDataUnit canData = CanData::getInstance().getDataFromDataSendQueue();
            if (canData.cmd == CAN_CMD_UPDATE_WEIGHT)
            {
                UpdateWeight(canData);
            }
            else if (canData.cmd == CAN_CMD_ZEROCALIB_RESULT)
            {
                // CanManager::getInstance().send
            }
        }
        sleep(1);
    }
}

void CanComThread::UpdateWeight(CanDataUnit data)
{
    cout << "------CanComThread::UpdateWeight-----" << endl;
    float netWeight = 0;
    memcpy(&netWeight, data.content, sizeof(float));
    printf("------netWeight-------%f\n", netWeight);
    if (data.content != nullptr)
    {
        delete[] data.content;
        data.content = nullptr;
    }
    if (netWeight < 0)
        netWeight = 0;

    CanManager::getInstance().sendWeight(netWeight); // 发送CAN数据
}
