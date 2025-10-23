#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <string>
#include "CanManager.h"
#include "Base.h"
#include "DeviceConfig.h"
#include "CanFrame.h"
#include "CanDBC.h"
#include "UIData.h"
#include "DataDef.h"
#include "GlobalFlag.h"

#define CAN_READ_INTERVAL 500   // 单位：毫秒
#define DEFUALT_BAUDRATE 125000 // 单位：k

// static init
bool CanManager::bCANExit = false;

using namespace std;

/**
 * function defination
 */

CanManager::CanManager(/* args */)
{
    for (unsigned i = 0; i < DEFAULT_MAX_CAN_COUNT; i++)
    {
        canSocks[i] = NULL;
    }

    this->recvInterval = CAN_READ_INTERVAL; // 500ms
    this->receiving = false;
}

CanManager::~CanManager()
{
    // CAN
    for (unsigned i = 0; i < DEFAULT_MAX_CAN_COUNT; i++)
    {
        if (canSocks[i] != NULL)
        {
            if (canSocks[i]->isOpened())
            {
                canSocks[i]->closeCan();
            }

            delete canSocks[i];
            canSocks[i] = NULL;
        }
    }
}

/**
 * CAN
 * 用户单次发送ID为id的CAN报文
 */
bool CanManager::sendGeneralData(GeneralCanData canData)
{
    CanSock *canSock = canSocks[0];

    if (canSock == NULL)
    {
        long baudrate = DEFUALT_BAUDRATE;
        /*string br = DeviceConfig::getInstance().get_value("can", "baudrate");

        if (!br.empty())
        {
            baudrate = std::stol(br);
        }*/
	baudrate = 250000;

        COUT << "new can sock: " << baudrate << endl;
        canSock = new CanSock(0, baudrate);
        canSocks[0] = canSock;

        // config & open
        canSock->config(baudrate);
        int ret = canSock->openCan();

        if (ret == -1)
        {
            COUT << "open can0 failed" << endl;
            return false;
        }
        else
        {
            COUT << "Open can0 OK!" << endl;
        }
    }

    // log output
    COUT << "data length to send: " << to_string(canData.length) << endl;
    for (int i = 0; i < canData.length; ++i)
    {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(canData.data[i]) << " ";
    }
    cout << endl;

    // sent to remote
    int wx = canSock->sendDataFrame((char *)canData.data, canData.length, canData.messageId, canData.isExtend);
    COUT << "send can frame result: " << to_string(wx) << endl;

    return true;
}

bool CanManager::startGeneralDataReceive(unsigned short interval)
{
    recvInterval = interval;

    if (receiving)
    {
        return true;
    }

    COUT << "------ start can receiving ------" << endl;
    std::thread can(canReceiveFunction, this);
    receiving = true;

    can.join();

    return true;
}

bool CanManager::stopGeneralDataReceive()
{
    COUT << "------ stop can receiving ------" << endl;
    bCANExit = true;
    receiving = false;

    return true;
}

/**
 * @brief 间隔循环接收CAN数据处理函数
 *
 * @param manager
 * @return void*
 */
void *CanManager::canReceiveFunction(CanManager *manager)
{
    // 取sock
    CanSock *canSock = manager->canSocks[0];

    // loop
    while (!bCANExit)
    {
        // send frame data
        if (canSock == NULL)
        {
            long baudrate = DEFUALT_BAUDRATE;
            // string br = DeviceConfig::getInstance().get_value("can", "baudrate");

            // if (!br.empty())
            // {
            //     baudrate = std::stol(br);
            // }
            baudrate = 250; // 500k

            COUT << "------ new can sock: " << baudrate << " -------" << endl;
            canSock = new CanSock(0, baudrate);
            manager->canSocks[0] = canSock;

            // config & open
            canSock->config(baudrate);
            int ret = canSock->openCan();

            if (ret == -1)
            {
                COUT << "open can0 failed" << endl;
                return NULL;
            }
            else
            {
                COUT << "Open can0 OK!" << endl;
            }
        }

        // receive response
        uint32_t msgId;
        char buffer[64] = {0};
        size_t rx = canSock->recvData(buffer, &msgId);

        if (rx > 0)
        {
            COUT << "recv can frame result: " << to_string(rx) << " with id: " << msgId << endl;
            for (unsigned int i = 0; i < rx; ++i)
            {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(buffer[i]) << " ";
            }
            cout << endl;

            // parse it
            CanCmd cmd;
            cmd.messageId = msgId;
            cmd.canType = CAN_TYPE_CAN_20;
            manager->parseCanRecvGeneralData(cmd, buffer, rx);
        }

        usleep(manager->recvInterval * 1000);

        /*
        COUT << "------ can sending ----- " << endl;

        // test for send
        GeneralCanData data1;
        data1.data[0] = 0x01;
        data1.data[1] = 0x02;
        data1.data[2] = 0x03;
        data1.data[3] = 0x04;
        data1.data[4] = 0x05;
        data1.data[5] = 0x06;
        data1.data[6] = 0x07;
        data1.data[7] = 0x08;
        data1.isExtend = true;
        data1.length = 8;
        data1.messageId = 0x18FA0015;

        manager->sendGeneralData(data1);
        usleep(500000); */
    }

    return NULL;
}

/**
 * CAN
 * 用户发送ID为id的CAN报文，发送间隔周期（单位：ms）
 */
int CanManager::CANSendStart(uint32_t id, int cmdInterval)
{
    lock_guard<mutex> lock(canMutex);

    CanCmd canCmd;
    canCmd.messageId = id;
    canCmd.interval = cmdInterval;

    std::thread can(canThreadFunction, canCmd);
    can.detach();

    return 0;
}

/**
 * CAN
 * 某个CAN，发送消息
 */
int CanManager::CANSendStop(uint32_t id)
{
    bCANExit = true;
    return 0;
}

/**
 * CAN 发送指令
 */
void *CanManager::canThreadFunction(CanCmd canCmd)
{
    CanManager &obj = CanManager::getInstance();

    //
    CanSock *canSock = obj.canSocks[0];

    // loop
    while (!bCANExit)
    {
        // send frame data
        if (canSock == NULL)
        {
            long baudrate = DEFUALT_BAUDRATE;
            string br = DeviceConfig::getInstance().get_value("can", "baudrate");

            if (!br.empty())
            {
                baudrate = std::stol(br);
            }

            COUT << "new can sock: " << baudrate << endl;
            canSock = new CanSock(0, baudrate);
            obj.canSocks[0] = canSock;

            // config & open
            canSock->config(baudrate);
            int ret = canSock->openCan();

            if (ret == -1)
            {
                COUT << "open can0 failed" << endl;
                return NULL;
            }
            else
            {
                COUT << "Open can0 OK!" << endl;
            }
        }

        // later send data
        CanFrame *canFrame = new CanFrame(); // TODO: need make instance with data
        uint8_t length = canFrame->getRealFrameLength();
        uint8_t *data = canFrame->getData();

        // log out data to be sent
        COUT << "data length to send: " << to_string(length) << endl;
        for (int i = 0; i < length; ++i)
        {
            cout << hex << setw(2) << setfill('0') << static_cast<int>(data[i]) << " ";
        }
        cout << endl;

        // sent to remote
        int wx = canSock->sendDataFrame((char *)data, length, canCmd.messageId, canFrame->isExtend);
        COUT << "send can frame result: " << to_string(wx) << endl;

        // 计次执行发送
        delete canFrame;

        // receive can frame data
        usleep(10000);

        // receive response
        uint32_t msgId;
        char buffer[64] = {0};
        size_t rx = canSock->recvData(buffer, &msgId);

        if (rx > 0)
        {
            COUT << "recv can frame result: " << to_string(rx) << " with id: " << msgId << endl;
            for (unsigned int i = 0; i < rx; ++i)
            {
                cout << hex << setw(2) << setfill('0') << static_cast<int>(buffer[i]) << " ";
            }
            cout << endl;

            // parse it
            CanCmd cmd;
            cmd.messageId = msgId;
            cmd.canType = CAN_TYPE_CAN_20;
            obj.parseCanRecvData(cmd, buffer, rx);
        }

        usleep(canCmd.interval * 1000);
    }

    return NULL;
}

/**
 * parse the can data frame
 */
bool CanManager::parseCanRecvGeneralData(CanCmd &canCmd, char *data, size_t length)
{
    // TODO
    int msgType = data[0];

    if (msgType == 1)
    {
        UIDataUnit dataUnit;
        dataUnit.cmd = UI_MSG_CMD_ZEROCALIB;
        UIData::getInstance().addDataToDataRecvQueue(dataUnit); // 执行UI的全部置0操作
    }

    return true;
}

/**
 * parse the can data frame
 */
bool CanManager::parseCanRecvData(CanCmd &canCmd, char *data, size_t length)
{
    // TODO : instance?
    CanDBC *canDBC = new CanDBC();
    CanFrame frame(canCmd.canType);

    frame.setData((uint8_t *)data, length);
    frame.length = length;
    frame.id = canCmd.messageId;

    std::vector<CanSignal *> *signals = canDBC->parseFrame(frame);

    // output all signals
    if (signals != NULL && signals->size() > 0)
    {
        COUT << "recv can signal result: " << to_string(signals->size()) << endl;
        for (unsigned int i = 0; i < signals->size(); ++i)
        {
            CanSignal *signal = signals->at(i);
            cout << hex << setw(2) << setfill('0') << static_cast<int>(signal->value) << " ";

            // clear
            delete signal;
        }
        cout << endl;

        delete signals;
    }

    delete canDBC;
    return true;
}

/**
 * tools function here
 */
int CanManager::hexStringToNumber(const std::string &hexString)
{
    int number;
    std::istringstream iss(hexString);
    iss >> std::hex >> number;
    return number;
}

uint16_t CanManager::crc16(uint8_t *data, int length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

std::string CanManager::toHexString(int num)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << std::hex << num;
    return oss.str();
}

/**
 * 模拟lua调用can循环发送
 */
void CanManager::testCan()
{
    // CANSendStart(752, 1, 100, 500, "SLControl2", false);
    // CANSendStart(751, 1, -1, 2000, "SLControl1", false);
    // sleep(500);
    // CANSendStop(751);
    // sleep(20);
    // CANSendStop(752);
}

void CanManager::sendZeroCalibResult(uint8_t result, uint8_t sensorid)
{
    GeneralCanData sendData;
    uint8_t can_data[] = {CAN_MSG_TYPE_ZERO_RESULT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    can_data[1] = result;
    can_data[2] = 0;
    can_data[3] = sensorid;

    for (int i = 0; i < 8; i++)
    {
        sendData.data[i] = can_data[i];
    }

    sendData.isExtend = true;
    sendData.length = 8;
    sendData.messageId = TX_XUGONG_CAN_MSG_ID;
    sendGeneralData(sendData);
}

void CanManager::sendWeight(double totalWeigh)
{
    GeneralCanData sendData;
    uint8_t can_data[] = {CAN_MSG_TYPE_WEIGHT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // weight value
    int weight = totalWeigh / 100; // 单位：0.1吨
    can_data[1] = weight & 0xFF;
    can_data[2] = (weight >> 8) & 0xFF;

    // edge computing?
    can_data[3] = (GlobalFlag::getInstance().bEdgeComputeMode ? 1 : 2); // 1:边缘计算,2:云计算

#ifdef EXCEPTION_REPORT
    // exception code
    can_data[4] = GlobalFlag::getInstance().exceptionCode;
    can_data[5] = GlobalFlag::getInstance().expGC31s;
    can_data[6] = GlobalFlag::getInstance().expSensors;
#endif
    for (int i = 0; i < 8; i++)
    {
        sendData.data[i] = can_data[i];
    }

    sendData.isExtend = true;
    sendData.length = 8;
    sendData.messageId = TX_XUGONG_CAN_MSG_ID;
    sendGeneralData(sendData);
}

void CanManager::startLoop()
{
    canComThread.start();
	//startGeneralDataReceive(500);
    //canComThread.start();
}

void CanManager::stopLoop()
{
    // canComThread.stop();
    stopGeneralDataReceive();
    canComThread.join();
}
