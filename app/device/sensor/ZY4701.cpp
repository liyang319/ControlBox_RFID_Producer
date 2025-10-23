#include "ZY4701.h"
#include "DataDef.h"
#include "Base.h"
#include "Utility.h"
#include <math.h>
#include "SensorData.h"
#include "GlobalFlag.h"

#define DEFAULT_CMD_BUFFERSIZE 50
#define MAX_TRY_CONNECT_TIME 3
#define ZY4701_CH1_SETTING_ADDR 0x0100
#define ZY4701_CH2_SETTING_ADDR 0x0200
#define ZY4701_FIRMWARE_ADDR 0x0082
#define ZY4701_MEASUREDATA_ADDR 0x0089

#define ZY4701_CH1_GAIN_ADDR 0x0101
#define ZY4701_CH2_GAIN_ADDR 0x0201

#define ZY4701_CH1_FILTYP_ADDR 0x0103
#define ZY4701_CH2_FILTYP_ADDR 0x0203

#define ZY4701_CH1_SETTING_READLEN 14
#define ZY4701_CH2_SETTING_READLEN 14
#define ZY4701_CH1_FIRMARE_READLEN 7
#define ZY4701_CH1_MEASURE_READLEN 9

#define ZY4701_SINGLE_REG16_LEN 1
#define ZY4701_SINGLE_REG32_LEN 2
#define ZY4701_FILTYP_LEN 1

#define ZY4701_CMD_SETREG_SINGLE 0x06
#define ZY4701_CMD_SETREG_MULTI 0x10
#define ZY4701_CMD_READREG_MULTI 0x03

#define ZY4701_RESPONSE_READREG_SINGLE_LEN 8

ZY4701::ZY4701() : initialized(false)
{
    // init();
}

void ZY4701::initTimer(map<uint8_t, SensorDataUnit> sensorDataMap)
{
    COUT << "------Z4701::initTimer-------" << endl;
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

bool ZY4701::updateSensorTimer(uint8_t devAddr)
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

int ZY4701::getSensorIDByAddr(uint8_t devAddr)
{
    int sensorID = -1;
    auto it = sensorMap.find(devAddr);
    if (it != sensorMap.end())
    {
        sensorID = it->second.sensorID;
    }
    return sensorID;
}

bool ZY4701::checkTimeout(std::chrono::steady_clock::time_point currentTime, uint8_t &exception)
{
    bool bSuccess = true;
    exception = 0;
    for (auto it = sensorMap.begin(); it != sensorMap.end(); ++it)
    {
        int sensorID = it->second.sensorID;
        auto lastTime = it->second.lastTime;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        if (duration > DEFAULT_ZY4701_TIMEOUT)
        {
            // COUT << "------timeout-------" << std::dec << it->second.sensorID << endl;
            exception = exception | 1 << (sensorID - 1);
            bSuccess = false;
        }
    }
    return bSuccess;
}

void ZY4701::init()
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

ZY4701::~ZY4701()
{
    // 析构函数实现
    // 可以在这里进行资源释放等操作
}

void ZY4701::connectSerialPort()
{
    Rs485::connectSerialPort(SENSOR_PORT);
}

uint16_t ZY4701::getCrc(unsigned char *buf, int len)
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

int ZY4701::readData(uint8_t *rxData, int len)
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

int ZY4701::sendData(uint8_t *data, int len)
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

bool ZY4701::readMeasureData(unsigned char devAddr, double &tempVal, uint16_t *chlVal)
{
    cout << "---------ZY4701::readMeasureData-------begin--" << endl;
    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = 0x00;
    comUnit.protocal = _WSENSOR_PROTOCL_ZY4701;
    comUnit.cmdType = CODE_ZY4701_CMD_READATA;
    comUnit.cmd[0] = devAddr;
    comUnit.cmd[1] = 0x03;
    comUnit.cmd[2] = (ZY4701_MEASUREDATA_ADDR >> 8) & 0xFF;
    comUnit.cmd[3] = ZY4701_MEASUREDATA_ADDR & 0xFF;
    comUnit.cmd[4] = 0x00;
    comUnit.cmd[5] = ZY4701_CH1_MEASURE_READLEN;
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
    // SensorData::getInstance().addDataToDataSendQueue(comUnit);
    // usleep(10000);
    // return bSuccess;
    ////////////////////////////////////////////////
    int sendLen = writeDataToPort((char *)comUnit.cmd, comUnit.len);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to Z4071!" << endl;
        return false;
    }
    int timeout = DEFAULT_ZY4701_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----readMeasureData----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            // if (GlobalFlag::getInstance().bSensorDataLogOn)
            // {
            //     std::cout << "[RecvData size=" << std::dec << rxLen << "]: ";
            //     for (int i = 0; i < rxLen; i++)
            //     {
            //         // fflush(stdout);
            //         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            //     }
            //     std::cout << "" << endl;
            // }
            // if (rxData[0] == devAddr && rxLen == 23 && rxData[2] == 18) // ZY4701_CH1_MEASURE_READLEN * 2 + 5 = 23
            // {
            //     uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
            //     uint8_t crcHigh = crcVal & 0x00ff;
            //     uint8_t crcLow = (crcVal >> 8) & 0x00ff;
            //     std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
            //     if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
            //     {
            //         uint16_t temp = 0;
            //         temp = ((static_cast<uint16_t>(rxData[3]) << 8) | static_cast<uint16_t>(rxData[4]));
            //         tempVal = (double)temp / 100;
            //         chlVal[0] = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6]));
            //         chlVal[1] = ((static_cast<uint16_t>(rxData[7]) << 8) | static_cast<uint16_t>(rxData[8]));
            //         std::cout << "tempVal: " << std::dec << static_cast<int>(tempVal) << std::endl;
            //         std::cout << "chlVal[0]: " << std::dec << static_cast<int>(chlVal[0]) << "   chlVal[1]: " << std::dec << static_cast<int>(chlVal[1]) << std::endl;
            //         bSuccess = true;
            //     }
            //     else
            //     {
            //         COUT << "Invalid CRC" << endl;
            //     }
            // }
            bSuccess = processGetMeasureData(rxData, rxLen, devAddr, tempVal, chlVal);
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    cout << "---------ZY4701::readMeasureData-------end--" << endl;
    return bSuccess;
}

bool ZY4701::processGetMeasureData(uint8_t *rxData, int rxLen, uint8_t devAddr, double &tempVal, uint16_t *chlVal)
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

    if (rxData[0] == devAddr && rxLen == 23 && rxData[2] == 18) // ZY4701_CH1_MEASURE_READLEN * 2 + 5 = 23
    {
        uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
        uint8_t crcHigh = crcVal & 0x00ff;
        uint8_t crcLow = (crcVal >> 8) & 0x00ff;
        std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
        {
            uint16_t temp = 0;
            temp = ((static_cast<uint16_t>(rxData[3]) << 8) | static_cast<uint16_t>(rxData[4]));
            tempVal = (double)temp / 100;
            chlVal[0] = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6]));
            chlVal[1] = ((static_cast<uint16_t>(rxData[7]) << 8) | static_cast<uint16_t>(rxData[8]));
            std::cout << "tempVal: " << std::dec << static_cast<int>(tempVal) << std::endl;
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

bool ZY4701::getDeviceSN(uint8_t devAddr, string &strSN, uint16_t &ver)
{
    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_ZY4701_CMD_READATA;
    comUnit.cmd[0] = devAddr;
    comUnit.cmd[1] = 0x03;
    comUnit.cmd[2] = (ZY4701_FIRMWARE_ADDR >> 8) & 0xFF;
    comUnit.cmd[3] = ZY4701_FIRMWARE_ADDR & 0xFF;
    comUnit.cmd[4] = 0x00;
    comUnit.cmd[5] = ZY4701_CH1_FIRMARE_READLEN;
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
    // SensorData::getInstance().addDataToDataSendQueue(comUnit);
    // usleep(10000);
    // return bSuccess;
    ////////////////////////////////////////////////
    int sendLen = writeDataToPort((char *)comUnit.cmd, comUnit.len);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to Z4071!" << endl;
        return false;
    }
    int timeout = DEFAULT_ZY4701_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----readSensorSN----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            // std::cout << "[RecvData size=" << rxLen << "]: ";
            // for (int i = 0; i < rxLen; i++)
            // {
            //     // fflush(stdout);
            //     std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            // }
            // std::cout << "" << endl;
            // int validLen = rxLen - 5;
            // std::string sn = Utility::hexToString2(rxData + 3, validLen);
            // std::cout << "SN: " << sn << std::endl;
            bSuccess = processGetDeviceSN(rxData, rxLen, devAddr, strSN, ver);
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool ZY4701::processGetDeviceSN(uint8_t *rxData, int rxLen, uint8_t devAddr, string &strSN, uint16_t &ver)
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

    if (rxData[0] == devAddr && rxLen == 19 && rxData[2] == 14) // ZY4701_CH1_FIRMARE_READLEN * 2 + 5 = 19
    {
        uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
        uint8_t crcHigh = crcVal & 0x00ff;
        uint8_t crcLow = (crcVal >> 8) & 0x00ff;
        std::cout << "GetDeviceSN crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
        {
            int validLen = rxLen - 7;
            strSN = Utility::hexToString2(rxData + 3, validLen);
            ver = ((static_cast<uint16_t>(rxData[rxLen - 4]) << 8) | static_cast<uint16_t>(rxData[rxLen - 3]));
            COUT << "SN: " << strSN << "   ver:" << ver << std::endl;
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool ZY4701::getDeviceSetting(unsigned char devAddr, int chlID, uint16_t &gainVal, uint16_t &reverseVal, uint16_t &filterVal, uint16_t &zeroVal)
{
    COUT << "--------------ZY4701::getDeviceSetting------------begin--" << endl;
    uint16_t addr = 0;
    if (chlID == 0)
    {
        addr = ZY4701_CH1_SETTING_ADDR;
    }
    else
    {
        addr = ZY4701_CH2_SETTING_ADDR;
    }

    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_ZY4701_CMD_READATA;
    comUnit.cmd[0] = devAddr;
    comUnit.cmd[1] = 0x03;
    comUnit.cmd[2] = (addr >> 8) & 0xFF;
    comUnit.cmd[3] = addr & 0xFF;
    comUnit.cmd[4] = 0x00;
    comUnit.cmd[5] = ZY4701_CH1_SETTING_READLEN;
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
    // SensorData::getInstance().addDataToDataSendQueue(comUnit);
    // usleep(10000);
    // return bSuccess;
    ////////////////////////////////////////////////
    int sendLen = writeDataToPort((char *)comUnit.cmd, comUnit.len);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to Z4071!" << endl;
        return false;
    }
    int timeout = DEFAULT_ZY4701_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----getDeviceSetting----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            // if (GlobalFlag::getInstance().bSensorDataLogOn)
            // {
            //     std::cout << "[RecvData size=" << rxLen << "]: ";
            //     for (int i = 0; i < rxLen; i++)
            //     {
            //         // fflush(stdout);
            //         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            //     }
            //     std::cout << "" << endl;
            // }
            // if (rxData[0] == devAddr && rxLen == 33 && rxData[2] == 28) // ZY4701_CH1_SETTING_READLEN * 2 + 5 = 33
            // {
            //     uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
            //     uint8_t crcHigh = crcVal & 0x00ff;
            //     uint8_t crcLow = (crcVal >> 8) & 0x00ff;
            //     std::cout << "getDeviceSetting crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
            //     if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
            //     {
            //         uint16_t gainVal = 0;
            //         gainVal = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6]));
            //         uint16_t zeroVal = 0;
            //         zeroVal = ((static_cast<uint16_t>(rxData[15]) << 8) | static_cast<uint16_t>(rxData[16]));
            //         std::cout << "gainVal: " << std::dec << static_cast<int>(gainVal) << std::endl;
            //         std::cout << "zeroVal: " << std::dec << static_cast<int>(zeroVal) << std::endl;
            //         bSuccess = true;
            //     }
            //     else
            //     {
            //         COUT << "Invalid CRC" << endl;
            //     }
            // }
            bSuccess = processGetDeviceSetting(rxData, rxLen, devAddr, chlID, gainVal, reverseVal, filterVal, zeroVal);
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    COUT << "--------------ZY4701::getDeviceSetting------------end--" << endl;
    return bSuccess;
}

bool ZY4701::processGetDeviceSetting(uint8_t *rxData, int rxLen, uint8_t devAddr, int chlID, uint16_t &gainVal, uint16_t &reverseVal, uint16_t &filterVal, uint16_t &zeroVal)
{
    bool bSuccess = false;
    int sensorID = 0;
    // if (chlID == 0)
    // {
    //     sensorID = 0;
    // }
    // else
    // {
    //     sensorID = 1;
    // }

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

    if (rxData[0] == devAddr && rxLen == 33 && rxData[2] == 28) // ZY4701_CH2_SETTING_READLEN * 2 + 5 = 19
    {
        uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
        uint8_t crcHigh = crcVal & 0x00ff;
        uint8_t crcLow = (crcVal >> 8) & 0x00ff;
        std::cout << "GetDeviceSetting crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow)
        {
            gainVal = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6]));
            reverseVal = ((static_cast<uint16_t>(rxData[7]) << 8) | static_cast<uint16_t>(rxData[8]));
            filterVal = ((static_cast<uint16_t>(rxData[9]) << 8) | static_cast<uint16_t>(rxData[10]));
            zeroVal = ((static_cast<uint16_t>(rxData[15]) << 8) | static_cast<uint16_t>(rxData[16]));
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    COUT << "gainVal: " << gainVal << std::endl;
    COUT << "reverseVal: " << reverseVal << std::endl;
    COUT << "filterVal: " << filterVal << std::endl;
    COUT << "zeroVal: " << zeroVal << std::endl;
    return bSuccess;
}

bool ZY4701::setSingleReg(unsigned char devAddr, uint16_t regAddr, uint16_t regVal)
{
    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = ZY4701_CMD_SETREG_SINGLE;
    comUnit.cmd[0] = devAddr;
    comUnit.cmd[1] = ZY4701_CMD_SETREG_SINGLE;
    comUnit.cmd[2] = (regAddr >> 8) & 0xFF;
    comUnit.cmd[3] = regAddr & 0xFF;
    comUnit.cmd[4] = (regVal >> 8) & 0xFF;
    comUnit.cmd[5] = regVal & 0xFF;
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
    // SensorData::getInstance().addDataToDataSendQueue(comUnit);
    // usleep(10000);
    // return bSuccess;
    ////////////////////////////////////////////////
    int sendLen = writeDataToPort((char *)comUnit.cmd, comUnit.len);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to Z4071!" << endl;
        return false;
    }
    int timeout = DEFAULT_ZY4701_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----setSingleReg----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            bSuccess = processSetSingleReg(rxData, rxLen, devAddr, regAddr);
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool ZY4701::processSetSingleReg(uint8_t *rxData, int rxLen, uint8_t devAddr, uint16_t regAddr)
{
    bool bSuccess = false;
    int sensorID = 0;

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

    if (rxData[0] == devAddr && rxData[1] == ZY4701_CMD_SETREG_SINGLE && rxLen == ZY4701_RESPONSE_READREG_SINGLE_LEN)
    {
        uint16_t crcVal = getCrc((uint8_t *)rxData, rxLen - 2);
        uint8_t crcHigh = crcVal & 0x00ff;
        uint8_t crcLow = (crcVal >> 8) & 0x00ff;
        uint16_t rAddr = ((static_cast<uint16_t>(rxData[2]) << 8) | static_cast<uint16_t>(rxData[3])); // 接收到的寄存器地址
        std::cout << "processSetSingleReg crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if ((rAddr == regAddr) && (rxData[rxLen - 2] == crcHigh && rxData[rxLen - 1] == crcLow))
        {
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool ZY4701::setGainParam(unsigned char devAddr, int chlID, uint16_t gainVal)
{
    uint16_t addr = 0;
    if (chlID == 0)
    {
        addr = ZY4701_CH1_GAIN_ADDR;
    }
    else
    {
        addr = ZY4701_CH2_GAIN_ADDR;
    }

    bool bSuccess = false;
    bSuccess = setSingleReg(devAddr, addr, gainVal);
    return bSuccess;
}

bool ZY4701::setFilterParam(unsigned char devAddr, int chlID, uint16_t filterType)
{
    uint16_t addr = 0;
    if (chlID == 0)
    {
        addr = ZY4701_CH1_FILTYP_ADDR;
    }
    else
    {
        addr = ZY4701_CH2_FILTYP_ADDR;
    }

    bool bSuccess = false;
    bSuccess = setSingleReg(devAddr, addr, filterType);
    return bSuccess;
}
