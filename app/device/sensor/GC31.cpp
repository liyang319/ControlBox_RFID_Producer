#include "GC31.h"
#include "Base.h"
#include "Utility.h"
#include <math.h>
#include "SensorData.h"
#include "GlobalFlag.h"

#define DEFAULT_CMD_BUFFERSIZE 30
#define MAX_TRY_CONNECT_TIME 3

#define TR_B 3450

double calc_temp(uint16_t vt)
{
    double k = (double)vt / 4096.0;
    double r_thermo = k / (1 - k) * 10000;

    double n = log(r_thermo / 10000.0) / (double)TR_B;
    double v_temp = 1 / (n + 1.0 / 298.0) - 273.0;
    return v_temp;
}

GC31::GC31() : initialized(false)
{
    // init();
}

void GC31::initTimer(map<uint8_t, SensorDataUnit> sensorDataMap)
{
    COUT << "------GC31::initTimer-------" << endl;
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

bool GC31::updateSensorTimer(uint8_t devAddr)
{
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

bool GC31::checkTimeout(std::chrono::steady_clock::time_point currentTime, uint8_t &exception)
{
    bool bSuccess = true;
    exception = 0;
    for (auto it = sensorMap.begin(); it != sensorMap.end(); ++it)
    {
        int sensorID = it->second.sensorID;
        auto lastTime = it->second.lastTime;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();
        if (duration > DEFAULT_GC31_TIMEOUT)
        {
            // COUT << "------timeout-------" << std::dec << it->second.sensorID << endl;
            exception = exception | 1 << (sensorID - 1);
            bSuccess = false;
        }
    }
    return bSuccess;
}

int GC31::getSensorIDByAddr(uint8_t devAddr)
{
    int sensorID = -1;
    auto it = sensorMap.find(devAddr);
    if (it != sensorMap.end())
    {
        sensorID = it->second.sensorID;
    }
    return sensorID;
}

void GC31::init()
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

GC31::~GC31()
{
    // 析构函数实现
    // 可以在这里进行资源释放等操作
}

void GC31::connectSerialPort()
{
    Rs485::connectSerialPort(SENSOR_PORT);
}

unsigned char GC31::getCrc(unsigned char *dataBuf, int dataLen)
{
    unsigned char crc = 0;

    for (int i = 0; i < dataLen; i++)
    {
        crc += dataBuf[i];
    }

    return crc & 0xFF;
}

bool GC31::getDeviceSN(unsigned char devAddr)
{
    bool bSuccess = false;
    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_GET_SN;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_GET_SN;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 2);
    comUnit.cmd[3] = crcCmd;
    comUnit.cmd[4] = 0xfe;
    comUnit.len = 5;

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
}

bool GC31::readMeasureData(unsigned char devAddr)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_READ_DATA;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_READ_DATA;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 2);
    comUnit.cmd[3] = crcCmd;
    comUnit.cmd[4] = 0xfe;
    comUnit.len = 5;

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
}

bool GC31::setFilterParam(unsigned char devAddr, unsigned char filterType)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_SET_FILTER;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_SET_FILTER;
    comUnit.cmd[3] = filterType;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 3);
    comUnit.cmd[4] = crcCmd;
    comUnit.cmd[5] = 0xfe;
    comUnit.len = 6;

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
}

bool GC31::setEmptyLoadValue(unsigned char devAddr, uint16_t value)
{
    bool bSuccess = false;

    uint16_t divVal = value / 4;

    unsigned char lowVal = divVal & 0xff;
    unsigned char highVal = (divVal >> 8) & 0xff;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_SET_EMPTYLOAD;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_SET_EMPTYLOAD;
    comUnit.cmd[3] = highVal;
    comUnit.cmd[4] = lowVal;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 4);
    comUnit.cmd[5] = crcCmd;
    comUnit.cmd[6] = 0xfe;
    comUnit.len = 7;

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
}

bool GC31::setGainParam(unsigned char devAddr, uint16_t *gain)
{

    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_SET_GAIN;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_SET_GAIN;
    comUnit.cmd[3] = (uint8_t)gain[0];
    comUnit.cmd[4] = (uint8_t)gain[1];
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 4);
    comUnit.cmd[5] = crcCmd;
    comUnit.cmd[6] = 0xfe;
    comUnit.len = 7;

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
}

bool GC31::setReverseParam(unsigned char devAddr, unsigned char reverseVal)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_SET_REVERSE;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_SET_REVERSE;
    comUnit.cmd[3] = reverseVal;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 3);
    comUnit.cmd[4] = crcCmd;
    comUnit.cmd[5] = 0xfe;
    comUnit.len = 6;

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
}

bool GC31::getDeviceSetting(unsigned char devAddr)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_GET_SETTING;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_GET_SETTING;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 2);
    comUnit.cmd[3] = crcCmd;
    comUnit.cmd[4] = 0xfe;
    comUnit.len = 5;

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
}

bool GC31::resetDevice(unsigned char devAddr)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_RESET_DEVICE;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_RESET_DEVICE;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 2);
    comUnit.cmd[3] = crcCmd;
    comUnit.cmd[4] = 0xfe;
    comUnit.len = 5;

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
}

bool GC31::getVersion(unsigned char devAddr)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_GET_VERSION;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_GET_VERSION;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 2);
    comUnit.cmd[3] = crcCmd;
    comUnit.cmd[4] = 0xfe;
    comUnit.len = 5;

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
}

bool GC31::setAutoCalibration(unsigned char devAddr, unsigned char switchVal)
{
    bool bSuccess = false;

    SensorComUnit comUnit;
    comUnit.addr = devAddr;
    comUnit.cmdType = CODE_CMD_SET_AUTOCALIBRATION;
    comUnit.cmd[0] = 0xff;
    comUnit.cmd[1] = devAddr;
    comUnit.cmd[2] = CODE_CMD_SET_AUTOCALIBRATION;
    comUnit.cmd[3] = switchVal;
    unsigned char crcCmd = getCrc((uint8_t *)comUnit.cmd + 1, 3);
    comUnit.cmd[4] = crcCmd;
    comUnit.cmd[5] = 0xfe;
    comUnit.len = 6;

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
}

bool GC31::processGetDeviceSN(uint8_t *rxData, int rxLen, uint8_t &devAddr, std::string &strSN)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_GET_SN && rxLen == 13) // 检查包头包尾和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "getDeviceSN crc:   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            strSN = Utility::hexToString((unsigned char *)rxData + 3, rxLen - 5);
            COUT << "SN:" << strSN << endl;
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processGetMeasureData(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint16_t *chlVal, double *tempVal)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_READ_DATA && rxLen == 11) // 检查包头包尾和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            chlVal[0] = ((static_cast<uint16_t>(rxData[3]) << 8) | static_cast<uint16_t>(rxData[4])) * 4;
            chlVal[1] = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6])) * 4; // 数据x4，兼容陀螺仪数据
            uint16_t tempValShort = (static_cast<uint16_t>(rxData[7]) << 8) | static_cast<uint16_t>(rxData[8]);
            tempVal[0] = calc_temp(tempValShort);
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processSetFilterParam(uint8_t *rxData, int rxLen, uint8_t &devAddr)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_SET_FILTER && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "readMeasureData crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processSetEmptyLoadValue(uint8_t *rxData, int rxLen, uint8_t &devAddr)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_SET_EMPTYLOAD && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "setEmptyLoadValue crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processSetGainParam(uint8_t *rxData, int rxLen, uint8_t &devAddr)
{
    COUT << "-----------processSetGainParam------------" << endl;
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_SET_GAIN && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "setGainParam crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processSetReverseParam(uint8_t *rxData, int rxLen, uint8_t &devAddr)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_SET_REVERSE && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "setReverseParam crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processGetDeviceSetting(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint16_t *gain, uint16_t *reverseVal, uint16_t *filterType, uint16_t *autoCalibration)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_GET_SETTING && rxLen == 11) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "getDeviceSetting crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            gain[0] = rxData[4];
            gain[1] = rxData[5];
            reverseVal[0] = rxData[6];
            reverseVal[1] = rxData[6];
            filterType[0] = rxData[7];
            filterType[1] = rxData[7];
            autoCalibration[0] = rxData[8];
            autoCalibration[1] = rxData[8];
            bSuccess = true;
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processResetDevice(uint8_t *rxData, int rxLen, uint8_t &devAddr)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_RESET_DEVICE && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "resetDevice crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processGetVersion(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint8_t &ver)
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
    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_GET_VERSION && rxLen == 7) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "getVersion crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 4] == 0)
            {
                ver = rxData[rxLen - 3];
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

bool GC31::processSetAutoCalibration(uint8_t *rxData, int rxLen, uint8_t &devAddr)
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

    devAddr = rxData[1];
    if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[2] == CODE_RESPONSE_SET_AUTOCALIBRATION && rxLen == 6) // 检查包头包尾地址和长度
    {
        unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
        // std::cout << "setAutoCalibration crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
        if (rxData[rxLen - 2] == crcVal)
        {
            if (rxData[rxLen - 3] == 0)
            {
                bSuccess = true;
            }
        }
        else
        {
            COUT << "Invalid CRC" << endl;
        }
    }
    return bSuccess;
}

int GC31::readData(uint8_t *rxData, int len)
{
    int rxLen = -1;
    if (!isConnected())
    {
        // COUT << "GC31 serial not connected!!!" << endl;
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

int GC31::sendData(uint8_t *data, int len)
{
    if (!isConnected())
    {
        // COUT << "GC31 serial not connected!!!" << endl;
        return -1;
    }
    int sendLen = writeDataToPort((char *)data, len);
    COUT << "sendSenorData  len=" << sendLen << endl;
    return sendLen;
}

// bool GC31::getDeviceSN(unsigned char devAddr, string &strSN)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     strSN = "";
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_GET_SN, 0x00, 0xfe};
//     unsigned char crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----readSensorSN----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif
//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_GET_SN && rxLen == 13) // 检查包头包尾和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "getDeviceSN crc:   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     strSN = Utility::hexToString((unsigned char *)rxData + 3, rxLen - 5);
//                     COUT << "SN:" << strSN << endl;
//                     bSuccess = true;
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::readMeasureData(unsigned char devAddr, unsigned char *dataBuf, int &dataLen)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_READ_DATA, 0x00, 0xfe};
//     unsigned char crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----readMeasureData----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif
//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_READ_DATA && rxLen == 11) // 检查包头包尾和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     memcpy(dataBuf, rxData + 3, 6); // 赋值
//                     dataLen = 6;
//                     bSuccess = true;
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::readMeasureData(unsigned char devAddr, uint16_t *chlVal, double *tempVal)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_READ_DATA, 0x00, 0xfe};
//     unsigned char crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----readMeasureData----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif
//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_READ_DATA && rxLen == 11) // 检查包头包尾和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "readMeasureData crc :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     chlVal[0] = ((static_cast<uint16_t>(rxData[3]) << 8) | static_cast<uint16_t>(rxData[4])) * 4;
//                     chlVal[1] = ((static_cast<uint16_t>(rxData[5]) << 8) | static_cast<uint16_t>(rxData[6])) * 4;
//                     uint16_t tempValShort = (static_cast<uint16_t>(rxData[7]) << 8) | static_cast<uint16_t>(rxData[8]);
//                     tempVal[0] = calc_temp(tempValShort);
//                     bSuccess = true;
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::setFilterParam(unsigned char devAddr, unsigned char filterType)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_SET_FILTER, filterType, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 3);
//     cmdBuf[4] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----setFilterParam----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif
//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_SET_FILTER && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "readMeasureData crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::setEmptyLoadValue(unsigned char devAddr, uint16_t value)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char lowVal = value & 0xff;
//     unsigned char highVal = (value >> 8) & 0xff;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_SET_EMPTYLOAD, highVal, lowVal, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 4);
//     cmdBuf[5] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----setEmptyLoadValue----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_SET_EMPTYLOAD && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "setEmptyLoadValue crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::setGainParam(unsigned char devAddr, unsigned char *gain)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_SET_GAIN, gain[0], gain[1], 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 4);
//     cmdBuf[5] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----setGainParam----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_SET_GAIN && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "setGainParam crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::setReverseParam(unsigned char devAddr, unsigned char reverseVal)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_SET_REVERSE, reverseVal, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 3);
//     cmdBuf[4] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----setReverseParam----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_SET_REVERSE && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "setReverseParam crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::getDeviceSetting(unsigned char devAddr, unsigned char *gain, unsigned char *reverseVal, unsigned char *filterType, unsigned char *autoCalibration)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_GET_SETTING, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----getDeviceSetting----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_GET_SETTING && rxLen == 11) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "getDeviceSetting crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     gain[0] = rxData[4];
//                     gain[1] = rxData[5];
//                     reverseVal[0] = rxData[6];
//                     reverseVal[1] = rxData[6];
//                     filterType[0] = rxData[7];
//                     filterType[1] = rxData[7];
//                     autoCalibration[0] = rxData[8];
//                     autoCalibration[1] = rxData[8];
//                     bSuccess = true;
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::resetDevice(unsigned char devAddr)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_RESET_DEVICE, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----resetDevice----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_RESET_DEVICE && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "resetDevice crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::getVersion(unsigned char devAddr, unsigned char &ver)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_GET_VERSION, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 2);
//     cmdBuf[3] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----getVersion----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_GET_VERSION && rxLen == 7) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "getVersion crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 4] == 0)
//                     {
//                         ver = rxData[rxLen - 3];
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }

// bool GC31::setAutoCalibration(unsigned char devAddr, unsigned char switchVal)
// {
//     if (!isConnected())
//     {
//         COUT << "GC31 serial not connected!!!" << endl;
//         return false;
//     }
//     bool bSuccess = false;
//     unsigned char cmdBuf[] = {0xff, devAddr, CODE_CMD_SET_AUTOCALIBRATION, switchVal, 0x00, 0xfe};
//     uint16_t crcCmd = getCrc((uint8_t *)cmdBuf + 1, 3);
//     cmdBuf[4] = crcCmd;
// #ifdef ENABLE_GC31_DEBUG_INFO
//     std::cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
//     for (int i = 0; i < sizeof(cmdBuf); i++)
//     {
//         // fflush(stdout);
//         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
//     }
//     std::cout << "" << endl;
// #endif
//     int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
//     if (sendLen <= 0)
//     {
//         COUT << "Failed to send data to GC31!" << endl;
//         return false;
//     }
//     int timeout = DEFAULT_CMD_TIMEOUT;
//     auto start = std::chrono::steady_clock::now(); // 获取当前时间
//     while (true)
//     {
//         auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
//         auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
//         if (duration >= timeout)
//         {
//             COUT << "----setAutoCalibration----timeout" << endl;
//             break;
//         }
//         unsigned char rxData[DEFAULT_CMD_BUFFERSIZE] = {0};
//         int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
//         if (rxLen >= 0)
//         {
// #ifdef ENABLE_GC31_DEBUG_INFO
//             std::cout << "[RecvData size=" << rxLen << "]: ";
//             for (int i = 0; i < rxLen; i++)
//             {
//                 // fflush(stdout);
//                 std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
//             }
//             std::cout << "" << endl;
// #endif

//             if (rxData[0] == 0xfe && rxData[rxLen - 1] == 0xff && rxData[1] == devAddr && rxData[2] == CODE_RESPONSE_SET_AUTOCALIBRATION && rxLen == 6) // 检查包头包尾地址和长度
//             {
//                 unsigned char crcVal = getCrc((uint8_t *)rxData + 1, rxLen - 3);
//                 std::cout << "setAutoCalibration crc  :   " << std::hex << static_cast<int>(crcVal) << std::endl;
//                 if (rxData[rxLen - 2] == crcVal)
//                 {
//                     if (rxData[rxLen - 3] == 0)
//                     {
//                         bSuccess = true;
//                     }
//                 }
//                 else
//                 {
//                     COUT << "Invalid CRC" << endl;
//                 }
//             }
//             break;
//         }
//         usleep(10000); // 等待10毫秒
//     }
//     return bSuccess;
// }
