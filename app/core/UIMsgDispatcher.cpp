#include "UIMsgDispatcher.h"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "Base.h"
#include "AppData.h"
#include "Utility.h"
#include "Base.h"
#include "GC31.h"
#include "DeviceConfig.h"
#include <sstream>
#include "WeighData.h"
#include "DataDef.h"
#include "Dwin.h"
#include "UIData.h"

UIMsgDispatcher::UIMsgDispatcher()
{
}

void UIMsgDispatcher::dispatchMsg(uint8_t *data, int len)
{
    cout << "------dispatchMsg------" << len << endl;
    if (len < MIN_UI_MSG_LEN) // 指令长度检查
        return;
    int dataLen = data[2];
    if (dataLen + 3 != len) // 数据长度检查
        return;
    int keyDataLen = data[6] * 2;
    if (keyDataLen + 7 != len) // 键值长度检查
        return;

    uint16_t cmdAddr = (data[4] << 8) | data[5];
    uint16_t keyValue = (data[len - 2] << 8) | data[len - 1];

    std::cout << "cmdAddr: 0x" << std::hex << std::setw(4) << std::setfill('0') << cmdAddr << std::endl;
    std::cout << "keyValue: 0x" << std::hex << std::setw(4) << std::setfill('0') << keyValue << std::endl;

    if (cmdAddr == UI_ADDR_TARE_CONFIRM && keyValue == UI_KEY_TARE_CONFIRM)
    {
        processSetTareCmd();
    }
    else if (cmdAddr == UI_ADDR_ZEROCALIB_CONFIRM && keyValue == UI_KEY_ZEROCALIB)
    {
        processZeroCalibCmd();
    }
    else if (cmdAddr == UI_ADDR_RESET_MENU && keyValue == UI_KEY_RESET)
    {
        processResetDeviceCmd();
    }
    else if (cmdAddr == UI_ADDR_EXCEPTION_MENU && keyValue == UI_KEY_EXCEPTION)
    {
        processExceptionCmd();
    }
    else if (cmdAddr == UI_ADDR_EXCEPTION_PREVIOUS_PAGE && keyValue == UI_KEY_EXCEPTION_PREVIOUS)
    {
        processExceptionPreviousCmd();
    }
    else if (cmdAddr == UI_ADDR_EXCEPTION_NEXT_PAGE && keyValue == UI_KEY_EXCEPTION_NEXT)
    {
        processExceptionNextCmd();
    }
    else if (cmdAddr == UI_ADDR_LOCAL_MODE_SWITCH)
    {
        processSetLocalMode(data, len);
    }
    else if (cmdAddr == UI_ADDR_OFFLINE_MODE_SWITCH)
    {
        processSetOfflineMode(data, len);
    }
}

void UIMsgDispatcher::processZeroCalibCmd()
{
    COUT << "-----processZeroCalibCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_ZEROCALIB;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processResetDeviceCmd()
{
    COUT << "-----processResetDeviceCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_RESET_DEVICE;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processExceptionCmd()
{
    COUT << "-----processExceptionCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_EXCETION_GET;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processSetTareCmd()
{
    COUT << "-----processSetTareCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_SET_TARE;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processExceptionPreviousCmd()
{
    COUT << "-----processExceptionPreviousCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_EXCETION_PREVIOUS;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processExceptionNextCmd()
{
    COUT << "-----processExceptionNextCmd-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_EXCETION_NEXT;
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}

void UIMsgDispatcher::processSetLocalMode(uint8_t *data, int len)
{
    COUT << "-----processSetLocalMode-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_SET_LOCAL_MODE;
    dataUnit.content = new uint8_t[sizeof(uint16_t)];
    memcpy(dataUnit.content, data + len - 2, 2);
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
    
}

void UIMsgDispatcher::processSetOfflineMode(uint8_t *data, int len)
{
    COUT << "-----processSetOfflineMode-----" << endl;
    UIDataUnit dataUnit;
    dataUnit.cmd = UI_MSG_CMD_SET_OFFLINE_MODE;
    dataUnit.content = new uint8_t[sizeof(uint16_t)];
    memcpy(dataUnit.content, data + len - 2, 2);
    UIData::getInstance().addDataToDataRecvQueue(dataUnit);
}