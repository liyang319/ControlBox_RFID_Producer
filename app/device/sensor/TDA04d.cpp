#include "TDA04d.h"
#include "DataDef.h"
#include "Base.h"
#include "Utility.h"
#include <math.h>
#include "SensorData.h"
#include "GlobalFlag.h"

#define DEFAULT_CMD_BUFFERSIZE 30
#define MAX_TRY_CONNECT_TIME 3

#define TR_B 3450

TDA04d::TDA04d() : initialized(false)
{
    // init();
}

void TDA04d::initTimer(map<uint8_t, SensorDataUnit> sensorDataMap)
{
    COUT << "------TDA04d::initTimer-------" << endl;
    for (auto it = sensorDataMap.begin(); it != sensorDataMap.end(); ++it)
    {
        SensorDataUnit *pData = &(it->second);
        SensorDataTimerUnit timerUnit;
        timerUnit.sensorID = pData->sensorID;
        timerUnit.lastTime = std::chrono::steady_clock::now();
        if (pData->bActive)
        {
            sensorMap.insert(std::make_pair(pData->sensorAddr, timerUnit));
        }
    }

    // 遍历map并输出数据
    for (const auto &pair : sensorMap)
    {
        COUT << "Key: 0x" << std::hex << static_cast<int>(pair.first)
             << ", Time: " << pair.second.lastTime.time_since_epoch().count() << std::endl;
    }
}

bool TDA04d::updateSensorTimer(uint8_t devAddr)
{
    // cout << "--------updateSensorTimer--------" << std::hex << static_cast<int>(devAddr) << endl;
    auto it = sensorMap.find(devAddr);
    if (it != sensorMap.end())
    {
        it->second.lastTime = std::chrono::steady_clock::now(); // 更新时间
        return true;
    }
    else
    {
        // std::cout << "Key 0x" << std::hex << static_cast<int>(devAddr) << " not found." << std::endl;
        return false;
    }
}

int TDA04d::getSensorIDByAddr(uint8_t devAddr)
{
    int sensorID = -1;
    auto it = sensorMap.find(devAddr);
    if (it != sensorMap.end())
    {
        sensorID = it->second.sensorID;
    }
    return sensorID;
}

bool TDA04d::checkTimeout(std::chrono::steady_clock::time_point currentTime, uint8_t &exception)
{
    bool bSuccess = true;
    exception = 0;
    for (auto it = sensorMap.begin(); it != sensorMap.end(); ++it)
    {
        int sensorID = it->second.sensorID;
        auto lastTime = it->second.lastTime;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        if (duration > DEFAULT_TDA04D_TIMEOUT)
        {
            // COUT << "------timeout-------" << std::dec << it->second.sensorID << endl;
            exception = exception | 1 << (sensorID - 1);
            bSuccess = false;
        }
    }
    return bSuccess;
}

void TDA04d::init()
{
    if (initialized)
        return;
    tryCount = 0;
    while (!connected)
    {
        connectSerialPort();
        if (++tryCount > MAX_TRY_CONNECT_TIME)
            break;
        sleep(1);
    }
    if (connected)
        initialized = true;
}

TDA04d::~TDA04d()
{
    // 析构函数实现
    // 可以在这里进行资源释放等操作
}

void TDA04d::connectSerialPort()
{
    Rs485::connectSerialPort(SENSOR_PORT);
}

uint16_t TDA04d::getCrc(unsigned char *buf, int len)
{
    uint16_t crc = 0xFFFF; // 初始值
    for (int i = 0; i < len; i++)
    {
        crc ^= buf[i]; // 将当前字节与 CRC 进行异或运算
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {                  // 如果 CRC 的最低位为 1
                crc >>= 1;     // 右移一位
                crc ^= 0xA001; // 与多项式 0xA001 异或
            }
            else
            {
                crc >>= 1; // 右移一位
            }
        }
    }
    return crc; // 返回 CRC 值
}

int TDA04d::readData(uint8_t *rxData, int len)
{
    int rxLen = -1;
    if (!isConnected())
    {
        // COUT << "TDA04d serial not connected!!!" << endl;
        return -1;
    }

    rxLen = readDataFromPort((char *)rxData, len);
    if (rxLen > 0)
    {
        if (GlobalFlag::getInstance().bSensorDataLogOn)
        {
            cout << "[RecvData size=" << rxLen << "]: ";
            for (int i = 0; i < rxLen; i++)
            {
                // fflush(stdout);
                cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            }
            cout << "" << endl;
        }
    }
    else
    {
        ; // std::cout << "[Receive ERROR]" << std::endl;
    }
    return rxLen;
}

int TDA04d::sendData(uint8_t *data, int len)
{
    if (!isConnected())
    {
        // COUT << "TDA04d serial not connected!!!" << endl;
        return -1;
    }
    int sendLen = writeDataToPort((char *)data, len);
    COUT << "sendSenorData  len=" << sendLen << endl;
    return sendLen;
}

bool TDA04d::readMeasureData(unsigned char devAddr)
{
    cout << "---------TDA04d::readMeasureData---------" << endl;
    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = 0x00;
    comUnit.protocal = _WSENSOR_PROTOCL_4CHWEIGHT;
    comUnit.cmdType = CODE_TDA04D_CMD_READATA;
    comUnit.cmd[0] = 0x01;
    comUnit.cmd[1] = 0x03;
    comUnit.cmd[2] = 0x00;
    comUnit.cmd[3] = 0x50;
    comUnit.cmd[4] = 0x00;
    comUnit.cmd[5] = 0x02;
    uint16_t crcCmd = getCrc((uint8_t *)comUnit.cmd, 6);
    comUnit.cmd[6] = crcCmd & 0xFF;
    comUnit.cmd[7] = (crcCmd >> 8) & 0xFF;
    comUnit.len = 8;
    if (GlobalFlag::getInstance().bSensorDataLogOn)
    {
        cout << "[SendData size=" << comUnit.len << "]: ";
        for (int i = 0; i < comUnit.len; i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(comUnit.cmd[i]) << " ";
        }
        cout << "" << endl;
    }
    SensorData::getInstance().addDataToDataSendQueue(comUnit);
    usleep(10000);
    return bSuccess;
    ////////////////////////////////////////////////
    // int sendLen = writeDataToPort((char *)comUnit.cmd, comUnit.len);
    // if (sendLen <= 0)
    // {
    //     COUT << "Failed to send data to GC31!" << endl;
    //     return false;
    // }
    // int timeout = DEFAULT_TDA04D_TIMEOUT;
    // auto start = std::chrono::steady_clock::now(); // 获取当前时间
    // while (true)
    // {
    //     auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
    //     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
    //     if (duration >= timeout)
    //     {
    //         COUT << "----readSensorSN----timeout" << endl;
    //         break;
    //     }
    //     unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
    //     int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
    //     if (rxLen > 0)
    //     {
    //         std::cout << "[RecvData size=" << rxLen << "]: ";
    //         for (int i = 0; i < rxLen; i++)
    //         {
    //             // fflush(stdout);
    //             std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
    //         }
    //         std::cout << "" << endl;
    //         break;
    //     }
    //     usleep(10000); // 等待10毫秒
    // }
    // return bSuccess;
}

bool TDA04d::processGetMeasureData(uint8_t *rxData, int rxLen, uint16_t *chlVal)
{
    bool bSuccess = false;
    if (rxLen < 0)
        return false;

    if (GlobalFlag::getInstance().bSensorDataLogOn)
    {
        cout << "[RecvData size=" << rxLen << "]: ";
        for (int i = 0; i < rxLen; i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
        }
        cout << "" << endl;
    }

    if (rxData[0] == 0x01 && rxLen == 9) // 检查包头和长度
    {
        uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
        uint8_t crcHigh = crcVal & 0x00ff;
        uint8_t crcLow = (crcVal >> 8) & 0x00ff;
        std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
        {
            chlVal[0] = ((static_cast<uint16_t>(rxData[3]) << 8) | static_cast<uint16_t>(rxData[4]));
            chlVal[1] = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6]));
            std::cout << "chlVal[0]: " << std::dec << static_cast<int>(chlVal[0]) << "   chlVal[1]: " << std::dec << static_cast<int>(chlVal[1]) << std::endl;
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}
