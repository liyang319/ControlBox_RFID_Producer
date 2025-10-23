#include "SensorMsgDispatcher.h"
#include <iostream>
#include "Base.h"
#include "AppData.h"
#include "Utility.h"
#include "Base.h"
#include "GC31.h"
#include "TDA04d.h"
#include "DeviceConfig.h"
#include <sstream>
#include "WeighData.h"
#include "DataDef.h"
#include "GlobalFlag.h"
#include "DBExceptionData.h"

SensorMsgDispatcher::SensorMsgDispatcher()
{
}

bool SensorMsgDispatcher::getSensorParam(uint8_t *data, int len, uint8_t &addr, uint8_t &cmdType)
{
    addr = 0;
    cmdType = 0;
    if (len < MIN_SENSOR_RESPONSE_SIZE)
        return false;
    addr = data[1];
    cmdType = data[2];
    // std::cout << "addr: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(addr) << std::endl;
    // std::cout << "cmdType: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(addr) << std::endl;
    return true;
}

void SensorMsgDispatcher::dispatchMsg(uint8_t *data, int len)
{
    COUT << "------SensorMsgDispatcher::dispatchMsg------" << len << endl;
    if (len < MIN_SENSOR_RESPONSE_SIZE)
        return;
    if (!getSensorParam(data, len, msg_addr, msg_cmdType))
        return;
    COUT << "cmdType: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(msg_cmdType) << std::endl;
    switch (msg_cmdType)
    {
    case CODE_RESPONSE_GET_SN:
        processGetSNCmd(data, len);
        break;
    case CODE_RESPONSE_GET_VERSION:
        processGetVersionCmd(data, len);
        break;
    case CODE_RESPONSE_GET_SETTING:
        processGetSettingCmd(data, len);
        break;
    case CODE_RESPONSE_READ_DATA:
        processReadDataCmd(data, len);
        break;
    case CODE_RESPONSE_SET_FILTER:
        processSetFilterCmd(data, len);
        break;
    case CODE_RESPONSE_SET_GAIN:
        processSetGainCmd(data, len);
        break;
    case CODE_RESPONSE_SET_EMPTYLOAD:
        processSetEmptyloadCmd(data, len);
        break;
    case CODE_RESPONSE_SET_REVERSE:
        processSetReverseCmd(data, len);
        break;
    case CODE_RESPONSE_SET_AUTOCALIBRATION:
        processSetAutocalibrationCmd(data, len);
        break;
    case CODE_RESPONSE_RESET_DEVICE:
        processResetDeviceCmd(data, len);
        break;
    }
}

void SensorMsgDispatcher::dispatchMsg(SensorComUnit sensorComUnit)
{
    COUT << "------SensorMsgDispatcher::dispatchMsg------" << sensorComUnit.len << endl;
    if (sensorComUnit.len < MIN_SENSOR_RESPONSE_SIZE)
        return;
    if (sensorComUnit.protocal == _WSENSOR_PROTOCL_4CHWEIGHT) // TAD04d process functions
    {
        switch (sensorComUnit.cmdType)
        {
        case CODE_TDA04D_CMD_READATA:
            processReadDataCmd_TAD04d(sensorComUnit.cmd, sensorComUnit.len);
            break;
        default:
            break;
        }
    }
    else // GC31 process functions
    {
        if (!getSensorParam(sensorComUnit.cmd, sensorComUnit.len, msg_addr, msg_cmdType))
            return;
        COUT << "cmdType: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(msg_cmdType) << std::endl;
        switch (msg_cmdType)
        {
        case CODE_RESPONSE_GET_SN:
            processGetSNCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_GET_VERSION:
            processGetVersionCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_GET_SETTING:
            processGetSettingCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_READ_DATA:
            processReadDataCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_SET_FILTER:
            processSetFilterCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_SET_GAIN:
            processSetGainCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_SET_EMPTYLOAD:
            processSetEmptyloadCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_SET_REVERSE:
            processSetReverseCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_SET_AUTOCALIBRATION:
            processSetAutocalibrationCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        case CODE_RESPONSE_RESET_DEVICE:
            processResetDeviceCmd(sensorComUnit.cmd, sensorComUnit.len);
            break;
        }
    }
}

void SensorMsgDispatcher::processGetSNCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processGetSNCmd-------------" << endl;
    string strSN = "";
    uint8_t devAddr;
    GC31::getInstance().processGetDeviceSN(data, len, devAddr, strSN);
    WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].sensorSN = strSN;
}

void SensorMsgDispatcher::processReadDataCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processReadDataCmd-------------" << endl;
    uint8_t devAddr;
    uint16_t chlVal[SENSOR_MAX_NUM] = {0};
    double tempVal[SENSOR_MAX_NUM] = {0};
    GC31::getInstance().processGetMeasureData(data, len, devAddr, chlVal, tempVal);
    std::copy(chlVal, chlVal + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].chVal);
    std::copy(tempVal, tempVal + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].tempVal);
#ifdef EXCEPTION_REPORT
    int sensorID = GC31::getInstance().getSensorIDByAddr(devAddr);
    for (int i = 0; i < SENSOR_MAX_NUM; i++)
    {
        if (chlVal[i] < 100 || chlVal[i] > 14000)
        {
            COUT << "-------sensor data exception--------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(devAddr) << "---sensorID=" << sensorID << endl;
            GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_SENSOR_DATA;
            GlobalFlag::getInstance().expSensors = GlobalFlag::getInstance().expSensors | 1 << ((sensorID - 1) * 2 + i);
            COUT << "-------expSensors--------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            // DBExceptionData::getInstance().inserExceptionData();
        }
        else
        {
            COUT << "-------sensor data exception--clear------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(devAddr) << endl;
            GlobalFlag::getInstance().expSensors = GlobalFlag::getInstance().expSensors & (~(1 << ((sensorID - 1) * 2 + i)));
            if (GlobalFlag::getInstance().expSensors == 0)
            {
                GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_SENSOR_DATA);
                // DBExceptionData::getInstance().inserExceptionData();
            }
        }
    }
    DBExceptionData::getInstance().inserExceptionData();
#endif
}

void SensorMsgDispatcher::processSetFilterCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processSetFilterCmd-------------" << endl;
    uint8_t devAddr;
    bool bSuccess = GC31::getInstance().processSetFilterParam(data, len, devAddr);
    if (bSuccess)
    {
        WeighData::getInstance().notifySensorSettingChanged(devAddr, true);
    }
}

void SensorMsgDispatcher::processSetEmptyloadCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processSetEmptyloadCmd-------------" << endl;
    uint8_t devAddr;
    bool bSuccess = GC31::getInstance().processSetEmptyLoadValue(data, len, devAddr);
    if (bSuccess)
    {
        WeighData::getInstance().notifySensorSettingChanged(devAddr, true);
    }
}

void SensorMsgDispatcher::processSetGainCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processSetGainCmd-------------" << endl;
    uint8_t devAddr;
    bool bSuccess = GC31::getInstance().processSetGainParam(data, len, devAddr);
    if (bSuccess)
    {
        WeighData::getInstance().notifySensorSettingChanged(devAddr, true);
    }
}

void SensorMsgDispatcher::processSetReverseCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processSetReverseCmd-------------" << endl;
    uint8_t devAddr;
    bool bSuccess = GC31::getInstance().processSetReverseParam(data, len, devAddr);
    if (bSuccess)
    {
        WeighData::getInstance().notifySensorSettingChanged(devAddr, true);
    }
}

void SensorMsgDispatcher::processGetSettingCmd(uint8_t *data, int len)
{
    uint8_t devAddr;
    uint16_t gain[SENSOR_MAX_NUM] = {0};
    uint16_t reverseVal[SENSOR_MAX_NUM] = {0};
    uint16_t filterType[SENSOR_MAX_NUM] = {0};
    uint16_t autoCalibration[SENSOR_MAX_NUM] = {0};
    GC31::getInstance().processGetDeviceSetting(data, len, devAddr, gain, reverseVal, filterType, autoCalibration);
    std::copy(gain, gain + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].settings.gain);
    std::copy(reverseVal, reverseVal + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].settings.reverseFlag);
    std::copy(filterType, filterType + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].settings.filter);
    std::copy(autoCalibration, autoCalibration + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].settings.autoCalib);
    WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].bSettingUpdated = false;
    COUT << "----sensor----processGetSettingCmd------" << WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].bSettingUpdated << endl;
}

void SensorMsgDispatcher::processResetDeviceCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processResetDeviceCmd-------------" << endl;
    uint8_t devAddr;
    GC31::getInstance().processResetDevice(data, len, devAddr);
}

void SensorMsgDispatcher::processGetVersionCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processGetVersionCmd-------------" << endl;
    uint8_t devAddr;
    uint8_t ver;
    GC31::getInstance().processGetVersion(data, len, devAddr, ver);
    WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].settings.version = ver;
}

void SensorMsgDispatcher::processSetAutocalibrationCmd(uint8_t *data, int len)
{
    COUT << "----sensor----------processSetAutocalibrationCmd-------------" << endl;
    uint8_t devAddr;
    bool bSuccess = GC31::getInstance().processSetAutoCalibration(data, len, devAddr);
    if (bSuccess)
    {
        WeighData::getInstance().notifySensorSettingChanged(devAddr, true);
    }
}

void SensorMsgDispatcher::processReadDataCmd_TAD04d(uint8_t *data, int len)
{
    COUT << "----sensor----------processReadDataCmd_TAD04d-------------" << endl;
    uint16_t chlVal[SENSOR_MAX_NUM] = {0};
    uint8_t devAddr = DEFAULT_TDA04D_ADDR; // 变送器目前默认地址0x01, sensor.ini中，变送器地址必须是0x01;
    TDA04d::getInstance().processGetMeasureData(data, len, chlVal);
    std::copy(chlVal, chlVal + SENSOR_MAX_NUM, WeighData::getInstance().currentWeighData.sensorDataMap[devAddr].chVal);

#ifdef EXCEPTION_REPORT
    // int sensorID = TDA04d::getInstance().getSensorIDByAddr(devAddr);
    int sensorID = 1;
    for (int i = 0; i < SENSOR_MAX_NUM; i++)
    {
        if (chlVal[i] > 4000) // 数据异常阈值
        {
            COUT << "-------sensor data exception--------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(devAddr) << "---sensorID=" << sensorID << endl;
            GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode | EXCEPTION_SENSOR_DATA;
            GlobalFlag::getInstance().expSensors = GlobalFlag::getInstance().expSensors | 1 << ((sensorID - 1) * 2 + i);
            COUT << "-------expSensors--------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(GlobalFlag::getInstance().expSensors) << endl;
            // DBExceptionData::getInstance().inserExceptionData();
        }
        else
        {
            COUT << "-------sensor data exception--clear------" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(devAddr) << endl;
            GlobalFlag::getInstance().expSensors = GlobalFlag::getInstance().expSensors & (~(1 << ((sensorID - 1) * 2 + i)));
            if (GlobalFlag::getInstance().expSensors == 0)
            {
                GlobalFlag::getInstance().exceptionCode = GlobalFlag::getInstance().exceptionCode & (~EXCEPTION_SENSOR_DATA);
                // DBExceptionData::getInstance().inserExceptionData();
            }
        }
    }
    DBExceptionData::getInstance().inserExceptionData();
#endif
}
