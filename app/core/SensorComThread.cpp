#include "SensorComThread.h"
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
#include "SensorData.h"
#include "SensorMsgDispatcher.h"
#include "GlobalFlag.h"
#include "DataDef.h"
#include "DBExceptionData.h"
#include "TDA04d.h"

using namespace std;
using namespace rapidjson;

SensorComThread::SensorComThread() : m_running(false)
{
}

SensorComThread::~SensorComThread()
{
    stop();
}

void SensorComThread::init()
{
    sensorWorkerThread.start();
    // GC31::getInstance().init();
}

void SensorComThread::start()
{
    if (!m_running)
    {
        init();
        m_running = true;
        m_thread = std::thread(&SensorComThread::threadFunction, this);
    }
}

void SensorComThread::stop()
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

void SensorComThread::join()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

bool SensorComThread::getAddrFromCmd(uint8_t &devAddr, uint8_t *cmdBuf, int len)
{
    devAddr = 0;
    if (len < MIN_SENSOR_RESPONSE_SIZE)
        return false;
    devAddr = cmdBuf[1];
    return true;
}

void SensorComThread::threadFunction()
{
    cout << "---------SensorComThread::threadFunction-----------" << endl;
    uint8_t dataBuf[DEFAULT_GC31_DATA_BUFFER_SIZE] = {0};
    int loopIndex = 1;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (m_running)
    {
        if (loopIndex == 0)
            loopIndex = 1;
        if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
        {
            int rxLen = GC31::getInstance().readData(dataBuf, sizeof(dataBuf));
            if (rxLen > 0)
            {
                SensorComUnit comUnit;
                memcpy(comUnit.cmd, dataBuf, rxLen);
                getAddrFromCmd(comUnit.addr, dataBuf, rxLen);
                comUnit.len = rxLen;
                SensorData::getInstance().addDataToDataRecvQueue(comUnit);
                rxLen = -1;
                memset(dataBuf, sizeof(DEFAULT_GC31_DATA_BUFFER_SIZE), 0);
                GC31::getInstance().updateSensorTimer(comUnit.addr);
            }
            ////////////////检查是否超时
            auto currentTime = std::chrono::steady_clock::now();

            if (!GC31::getInstance().checkTimeout(currentTime, GlobalFlag::getInstance().expGC31s))
            {
                GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_GC31_COM;
                DBExceptionData::getInstance().inserExceptionData();
                // COUT << "-1---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
                // COUT << "-1---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
                // COUT << "-1---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            }
            else
            {
                if (GlobalFlag::getInstance().expGC31s == 0)
                {
                    GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_GC31_COM);
                    // DBExceptionData::getInstance().inserExceptionData();
                }
                // COUT << "-2---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
                // COUT << "-2---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
                // COUT << "-2---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            }
            ////////////////////////
            if (loopIndex % 2 == 0)
            {
                if (SensorData::getInstance().getDataSendQueueSize() > 0)
                {
                    SensorComUnit comData = SensorData::getInstance().getDataFromDataSendQueue();
                    GC31::getInstance().sendData(comData.cmd, comData.len);
                }
            }
        }
        else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
        {
            int rxLen = TDA04d::getInstance().readData(dataBuf, sizeof(dataBuf));
            if (rxLen > 0)
            {
                SensorComUnit comUnit;
                memcpy(comUnit.cmd, dataBuf, rxLen);
                comUnit.len = rxLen;
                SensorData::getInstance().addDataToDataRecvQueue(comUnit);
                rxLen = -1;
                memset(dataBuf, sizeof(DEFAULT_GC31_DATA_BUFFER_SIZE), 0);
                comUnit.addr = 0x01; // 与sensor.ini 传感器地址配置一致
                TDA04d::getInstance().updateSensorTimer(comUnit.addr);
                rxLen = -1;
            }
            ////////////////检查是否超时
            auto currentTime = std::chrono::steady_clock::now();

            if (!TDA04d::getInstance().checkTimeout(currentTime, GlobalFlag::getInstance().expGC31s))
            {
                GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_GC31_COM;
                DBExceptionData::getInstance().inserExceptionData();
                // COUT << "-1---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
                // COUT << "-1---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
                // COUT << "-1---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            }
            else
            {
                if (GlobalFlag::getInstance().expGC31s == 0)
                {
                    GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_GC31_COM);
                    // DBExceptionData::getInstance().inserExceptionData();
                }
                // COUT << "-2---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
                // COUT << "-2---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
                // COUT << "-2---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            }
            ////////////////////////
            if (loopIndex % 2 == 0)
            {
                if (SensorData::getInstance().getDataSendQueueSize() > 0)
                {
                    SensorComUnit comData = SensorData::getInstance().getDataFromDataSendQueue();
                    TDA04d::getInstance().sendData(comData.cmd, comData.len);
                }
            }
        }
        // if (rxLen > 0)
        // {
        //     SensorComUnit comUnit;
        //     memcpy(comUnit.cmd, dataBuf, rxLen);
        //     // getAddrFromCmd(comUnit.addr, dataBuf, rxLen);
        //     comUnit.len = rxLen;
        //     SensorData::getInstance().addDataToDataRecvQueue(comUnit);
        //     // rxLen = -1;
        //     memset(dataBuf, sizeof(DEFAULT_GC31_DATA_BUFFER_SIZE), 0);
        //     if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
        //     {
        //         getAddrFromCmd(comUnit.addr, dataBuf, rxLen);
        //         GC31::getInstance().updateSensorTimer(comUnit.addr);
        //     }
        //     else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
        //     {
        //         comUnit.addr = 0x01;
        //         TDA04d::getInstance().updateSensorTimer(comUnit.addr);
        //     }
        //     rxLen = -1;
        // }
        // ////////////////检查是否超时
        // auto currentTime = std::chrono::steady_clock::now();
        // // std::cout << "----check exception--" << endl;
        // if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
        // {
        //     if (!GC31::getInstance().checkTimeout(currentTime, GlobalFlag::getInstance().expGC31s))
        //     {
        //         GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_GC31_COM;
        //         DBExceptionData::getInstance().inserExceptionData();
        //         // COUT << "-1---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
        //         // COUT << "-1---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
        //         // COUT << "-1---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
        //     }
        //     else
        //     {
        //         if (GlobalFlag::getInstance().expGC31s == 0)
        //         {
        //             GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_GC31_COM);
        //             // DBExceptionData::getInstance().inserExceptionData();
        //         }
        //         // COUT << "-2---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
        //         // COUT << "-2---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
        //         // COUT << "-2---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
        //     }
        //     ////////////////////////
        //     if (loopIndex % 2 == 0)
        //     {
        //         if (SensorData::getInstance().getDataSendQueueSize() > 0)
        //         {
        //             SensorComUnit comData = SensorData::getInstance().getDataFromDataSendQueue();
        //             GC31::getInstance().sendData(comData.cmd, comData.len);
        //         }
        //     }
        // }
        // else if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_4CHWEIGHT)
        // {
        //     if (!TDA04d::getInstance().checkTimeout(currentTime, GlobalFlag::getInstance().expGC31s))
        //     {
        //         GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_GC31_COM;
        //         DBExceptionData::getInstance().inserExceptionData();
        //         // COUT << "-1---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
        //         // COUT << "-1---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
        //         // COUT << "-1---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
        //     }
        //     else
        //     {
        //         if (GlobalFlag::getInstance().expGC31s == 0)
        //         {
        //             GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_GC31_COM);
        //             // DBExceptionData::getInstance().inserExceptionData();
        //         }
        //         // COUT << "-2---exception--expGC31s--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expGC31s) << endl;
        //         // COUT << "-2---exception--exceptionCode--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().exceptionCode) << endl;
        //         // COUT << "-2---exception--expSensors--" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
        //     }
        //     ////////////////////////
        //     if (loopIndex % 2 == 0)
        //     {
        //         if (SensorData::getInstance().getDataSendQueueSize() > 0)
        //         {
        //             SensorComUnit comData = SensorData::getInstance().getDataFromDataSendQueue();
        //             TDA04d::getInstance().sendData(comData.cmd, comData.len);
        //         }
        //     }
        // }
        usleep(10000);
        loopIndex++;
    }
}
