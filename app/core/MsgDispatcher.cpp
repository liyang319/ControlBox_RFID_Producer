#include "MsgDispatcher.h"
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
#include "Dwin.h"
#include "DataFormater.h"
#include "DataDef.h"
#include "UIData.h"
#include "HttpUtility.h"
#include "GlobalFlag.h"
#include "IPC.h"
#include "MdtuProv.h"
#include "ZY4701.h"

bool isOtaPkg(std::string &fileName)
{
    // 检查文件名是否以"WeighBox_"开头
    bool startsWithWeighBox = fileName.rfind("WeighBox_", 0) == 0;
    // 检查文件名是否以".zip"结尾
    bool endsWithZip = fileName.size() >= 4 && fileName.compare(fileName.size() - 4, 4, ".zip") == 0;
    // 返回两个条件的与运算结果
    return startsWithWeighBox && endsWithZip;
}

bool isDwinPkg(std::string &fileName)
{
    // 检查文件名是否以"DWIN_"开头
    bool startsWithWeighBox = fileName.rfind("DWIN_", 0) == 0;
    // 检查文件名是否以".zip"结尾
    string fileExt = Utility::getFileExtension(fileName);
    cout << "-----fileExt----" << fileExt << endl;
    bool endsWithZip = fileExt == "zip";
    // 返回两个条件的与运算结果
    return startsWithWeighBox && endsWithZip;
}

bool isParamFile(std::string &fileName)
{
    return (fileName == "mdtuprov.json");
}

bool isConfigFile(std::string &fileName)
{
    return (fileName == "sensor.ini");
}

std::string convertToTwoDigits(int num)
{
    if (num < 10)
    {
        return "0" + std::to_string(num);
    }
    else
    {
        return std::to_string(num);
    }
}

void parseGainValue(std::string &str, unsigned char *pNum)
{
    std::stringstream ss(str);
    char delimiter;

    // 尝试提取第一个数字
    int temp1;
    if (!(ss >> temp1) || temp1 < 0 || temp1 > 255)
    {
        COUT << "Error: First number is not a valid unsigned char." << std::endl;
        return;
    }
    pNum[0] = static_cast<unsigned char>(temp1);

    // 检查分隔符
    if (!(ss >> delimiter) || delimiter != '/')
    {
        COUT << "Error: Invalid format. Expected '/' delimiter." << std::endl;
        return;
    }

    // 尝试提取第二个数字
    int temp2;
    if (!(ss >> temp2) || temp2 < 0 || temp2 > 255)
    {
        COUT << "Error: Second number is not a valid unsigned char." << std::endl;
        return;
    }
    pNum[1] = static_cast<unsigned char>(temp2);

    // 检查是否还有额外的字符
    char c;
    if (ss >> c)
    {
        COUT << "Error: Extra characters found after second number." << std::endl;
        return;
    }
}

MsgDispatcher::MsgDispatcher(std::string &json_data)
{
    COUT << "[Received StatusCmd] :  " << json_data << endl;
    m_document.Parse(json_data.c_str());
    if (m_document.HasParseError())
    {
        COUT << "json error" << endl;
    }
    sensorAddr = 0;
    sensorID = 0;
    sensorKey = "";
    getSensorAddr();
}

void MsgDispatcher::dispatchMsg()
{
    if (m_document.HasParseError())
    {
        COUT << "Failed to parse JSON data" << endl;
        return;
    }
    string cmdCode = "";
    string cmdName = "";
    string cmdType = "";
    if (m_document.HasMember("cmd"))
    {
        cmdCode = m_document["cmd"].GetString();
        // if (cmdCode != "registerset")
        // {
        //     return;
        // }

        if (m_document.HasMember("name"))
        {
            cmdName = m_document["name"].GetString();
        }
        if (m_document.HasMember("type"))
        {
            cmdType = m_document["type"].GetString();
        }
        // else
        // {
        //     return;
        // }
    }
    else
    {
        COUT << "Command not found in JSON" << endl;
        return;
    }

    if (cmdCode == MSG_CMD_REG_SET)
    {
        if (cmdName == MSG_CMD_SET_GAIN)
        {
            processSetGainCmd();
        }
        else if (cmdName == MSG_CMD_SET_FILTER)
        {
            processSetFilterCmd();
        }
        else if (cmdName == MSG_CMD_SET_EMPTYLOAD)
        {
            processSetEmptyLoadValueCmd();
        }
        else if (cmdName == MSG_CMD_SET_REVERSE)
        {
            processSetReverseCmd();
        }
        else if (cmdName == MSG_CMD_SET_AUTOCALIB)
        {
            processSetAutoCalibrationCmd();
        }
        else if (cmdName == MSG_CMD_RESET_DEVICE)
        {
            processResetDeviceCmd();
        }
        else if (cmdName == MSG_CMD_RESTART_DEVICE)
        {
            processRestartDeviceCmd();
        }
    }
    else if (cmdCode == MSG_CMD_SET_CACULATE_MODE)
    {
        processSetCaculateModeCmd();
    }
    else if (cmdCode == MSG_CMD_SHOW_WEIGHT)
    {
        processShowWeightCmd();
    }
    // else if (cmdCode == MSG_CMD_MODE_QUERY)
    // {
    //     processModeQueryCmd();
    // }
    else if (cmdCode == MSG_CMD_PARAMATERS)
    {
        if (cmdType == MSG_CMD_PARAM_INI)
        {
            processGetIni();
        }
        if (cmdType == MSG_CMD_PARAM_JSON)
        {
            processGetJson();
        }
        if (cmdType == MSG_CMD_MODE_QUERY)
        {
            processModeQueryCmd();
        }
        if (cmdType == MSG_CMD_PARAM_MODIF)
        {
            processGetModifParamCmd();
        }
        else if (cmdType == MSG_CMD_PARAM_CALIBRATION)
        {
            processGetCalibrationParamCmd();
        }
        else if (cmdType == MSG_CMD_PARAM_SENSOR)
        {
            processGetSensorParamCmd();
        }
        else if (cmdType == MSG_CMD_PARAM_ALGORITHM)
        {
            processGetAlgorithmParamCmd();
        }
        else if (cmdType == MSG_CMD_PARAM_DTU)
        {
            processGetDtuParamCmd();
        }
    }
    else if (cmdCode == MSG_CMD_DFU)
    {
        processDfuCmd();
    }
    else if (cmdCode == MSG_CMD_CRT)
    {
        processCrtCmd();
    }
}

bool MsgDispatcher::getSensorAddr()
{
    sensorID = 0;
    if (m_document.HasMember("sensorid"))
    {
        sensorID = m_document["sensorid"].GetInt();
    }
    else
    {
        return false;
    }
    sensorKey = "sensor" + convertToTwoDigits(sensorID);
    // cout << "key = " << strKey << endl;
    string strVal = DeviceConfig::getInstance().get_value("nodeid", sensorKey);
    if (strVal.empty())
    {
        return false;
    }
    // cout << "val = " << strVal << endl;
    // 将string转换为unsigned int
    unsigned int num = std::stoul(strVal);
    // 将unsigned int转换为unsigned char
    sensorAddr = static_cast<unsigned char>(num);
    COUT << "mqtt DeviceID: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sensorAddr) << std::endl;
    return true;
}

void MsgDispatcher::processSetFilterCmd()
{
    COUT << "-----processSetFilterCmd-----" << endl;
    uint16_t filterType = 0;
    if (m_document.HasMember("value"))
    {
        filterType = static_cast<uint16_t>(m_document["value"].GetInt());
    }
    else
    {
        return;
    }
    COUT << "mqtt filterType: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(filterType) << std::endl;
    bool bSuccess = false;
    if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
    {
        bSuccess = ZY4701::getInstance().setFilterParam(sensorAddr, 0, filterType);
        bSuccess = ZY4701::getInstance().setFilterParam(sensorAddr, 1, filterType);
        if (bSuccess)
        {
            WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
        }
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
    {
        bSuccess = GC31::getInstance().setFilterParam(sensorAddr, (uint8_t)filterType);
    }
    // if (bSuccess)
    // {
    //     WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
    // }
}

void MsgDispatcher::processSetEmptyLoadValueCmd()
{
    COUT << "-----processSetEmptyLoadValueCmd-----" << endl;
    uint16_t emptyLoadValue = 0;
    if (m_document.HasMember("value"))
    {
        emptyLoadValue = static_cast<uint16_t>(m_document["value"].GetInt());
    }
    else
    {
        return;
    }
    COUT << "mqtt emptyLoadValue: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(emptyLoadValue) << std::endl;
    if (sensorAddr == 0)
    {
        // bool bSuccess = GC31::getInstance().setEmptyLoadValue(sensorAddr, emptyLoadValue);
        for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
        {
            SensorDataUnit *pData = &(it->second);
            if (pData->bActive)
            {
                if (pData->protocal == _MSENSOR_PROTOCOL_GC31)
                {
                    cout << "-----" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pData->sensorAddr) << endl;
                    GC31::getInstance().setEmptyLoadValue(pData->sensorAddr, emptyLoadValue);
                    usleep(100000);
                }
            }
        }
    }
    else
    {
        bool bSuccess = GC31::getInstance().setEmptyLoadValue(sensorAddr, emptyLoadValue);
    }
    // if (bSuccess)
    // {
    //     WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
    // }
}

void MsgDispatcher::processSetGainCmd()
{
    COUT << "-----processSetGainCmd-----" << endl;
    uint16_t gainVal[SENSOR_MAX_NUM] = {0};
    if (m_document.HasMember("value"))
    {
        const rapidjson::Value &value = m_document["value"];
        if (value.IsInt())
        {
            // std::cout << "value is an integer: " << value.GetInt() << std::endl;
            int valGain = value.GetInt();
            gainVal[0] = static_cast<uint16_t>(valGain);
            gainVal[1] = static_cast<uint16_t>(valGain);
        }
        // else if (value.IsString())
        // {
        //     string strGain = value.GetString();
        //     // parseGainValue(strGain, gainVal);
        // }
    }
    else
    {
        return;
    }
    COUT << "mqtt gain1: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(gainVal[0]) << std::endl;
    COUT << "mqtt gain2: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(gainVal[1]) << std::endl;
    bool bSuccess = false;
    if (GlobalFlag::getInstance().sensorProtocol == _WSENSOR_PROTOCL_ZY4701)
    {
        bSuccess = ZY4701::getInstance().setGainParam(sensorAddr, 0, gainVal[0]);
        bSuccess = ZY4701::getInstance().setGainParam(sensorAddr, 1, gainVal[1]);
        if (bSuccess)
        {
            WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
        }
    }
    else if (GlobalFlag::getInstance().sensorProtocol == _MSENSOR_PROTOCOL_GC31)
    {
        bSuccess = GC31::getInstance().setGainParam(sensorAddr, gainVal);
    }
    // if (bSuccess)
    // {
    //     WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
    // }
}

void MsgDispatcher::processSetReverseCmd()
{
    COUT << "-----processSetReverseCmd-----" << endl;
    unsigned char reverseVal = 0;
    if (m_document.HasMember("value"))
    {
        reverseVal = static_cast<unsigned char>(m_document["value"].GetInt());
    }
    else
    {
        return;
    }
    COUT << "mqtt reverseVal: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(reverseVal) << std::endl;
    bool bSuccess = GC31::getInstance().setReverseParam(sensorAddr, reverseVal);
    // if (bSuccess)
    // {
    //     WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
    // }
}

void MsgDispatcher::processResetDeviceCmd()
{
    COUT << "-----processResetDeviceCmd-----" << endl;
    GC31::getInstance().resetDevice(sensorAddr);
}

void MsgDispatcher::processSetAutoCalibrationCmd()
{
    COUT << "-----processSetAutoCalibrationCmd-----" << endl;
    unsigned char autoCalibVal = 0;
    if (m_document.HasMember("value"))
    {
        autoCalibVal = static_cast<unsigned char>(m_document["value"].GetInt());
    }
    else
    {
        return;
    }
    COUT << "mqtt autoCalibVal: 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(autoCalibVal) << std::endl;
    bool bSuccess = GC31::getInstance().setAutoCalibration(sensorAddr, autoCalibVal);
    // if (bSuccess)
    // {
    //     WeighData::getInstance().notifySensorSettingChanged(sensorAddr, true);
    // }
}

void MsgDispatcher::processSetCaculateModeCmd()
{
    COUT << "-----processSetCaculateModeCmd-----" << endl;
    int localMode = 0;
    if (m_document.HasMember("edge_computing"))
    {
        localMode = m_document["edge_computing"].GetInt();
        GlobalFlag::getInstance().bEdgeComputeMode = localMode > 0 ? true : false;
        if (localMode == COMPUTE_MODE_EDGE_ALL)
        {
            GlobalFlag::getInstance().bReportFullData = true;
        }
        else
        {
            GlobalFlag::getInstance().bReportFullData = false;
        }
        COUT << "mqtt edge_computing =  " << localMode << endl;
        if (GlobalFlag::getInstance().bEdgeComputeMode)
        {
            cout << "-----clear-----weigh_data_record_queue----" << endl;
            WeighData::getInstance().weigh_data_record_queue.clear();
        }
        Dwin::getInstance().setIcon(UI_ADDR_MAIN_ICON1, localMode > 0 ? UI_ICON_INDEX_EDGE_MODE_ON : UI_ICON_INDEX_CLOUD_MODE_ON);
        Dwin::getInstance().setIcon(UI_ADDR_LOCAL_MODE_SWITCH, localMode > 0 ? UI_KEY_SWITCH_ON : UI_KEY_SWITCH_OFF);
        Utility::writeMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_EDGE, to_string(localMode)); // 保存在自定义配置文件中
    }
}

void MsgDispatcher::processShowWeightCmd()
{
    COUT << "-----processShowWeightCmd-----" << endl;
    int modeVal = 0;
    float weightVal = 0;
    if (m_document.HasMember("remote"))
    {
        modeVal = m_document["remote"].GetInt();
        if (!GlobalFlag::getInstance().bEdgeComputeMode && modeVal > 0)
        {
            if (m_document.HasMember("payload"))
            {
                weightVal = m_document["payload"].GetFloat() / 1000;
                // printf("--------payload-----%f----\n", weightVal);
                COUT << "mqtt payload=" << weightVal << endl;
                UIDataUnit uiData;
                uiData.cmd = UI_CMD_UPDATE_WEIGHT;
                uiData.content = new unsigned char[sizeof(float)];
                memcpy(uiData.content, (uint8_t *)&weightVal, sizeof(float));
                UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数值

                if (weightVal > WeighData::getInstance().loadCap)
                {
                    if (!WeighData::getInstance().bOverload)
                    {
                        UIDataUnit uiData;
                        uiData.cmd = UI_CMD_OVERLOAD_ALARM_ON;
                        UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                        WeighData::getInstance().bOverload = true;
                    }
                }
                else
                {
                    if (WeighData::getInstance().bOverload)
                    {
                        UIDataUnit uiData;
                        uiData.cmd = UI_CMD_OVERLOAD_ALARM_OFF;
                        UIData::getInstance().addDataToDataSendQueue(uiData); // 更新UI数
                        WeighData::getInstance().bOverload = false;
                    }
                }
            }
        }
    }
}

void MsgDispatcher::processModeQueryCmd()
{
    COUT << "-----processModeQueryCmd-----" << endl;
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
}

void MsgDispatcher::processRestartDeviceCmd()
{
    COUT << "-----processRestartDeviceCmd-----" << endl;
    Utility::rebootSystem();
}

void MsgDispatcher::processGetModifParamCmd()
{
    COUT << "-----processGetModifParamCmd-----" << endl;
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    reportUnit.content = DataFormater::FormatModifParamData();
    AppData::getInstance().addDataToDataSendQueue(reportUnit);
}

void MsgDispatcher::processGetCalibrationParamCmd()
{
    COUT << "-----processGetCalibrationParamCmd-----" << endl;
    for (int i = 1; i <= 2; i++)
    {
        MqttPublishUnit reportUnit = {};
        reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
        reportUnit.content = DataFormater::FormatCalibrationParamData(i);
        AppData::getInstance().addDataToDataSendQueue(reportUnit);
    }
}

void MsgDispatcher::processGetSensorParamCmd()
{
    COUT << "-----processGetSensorParamCmd-----" << endl;
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    reportUnit.content = DataFormater::FormatSensorParamData();
    AppData::getInstance().addDataToDataSendQueue(reportUnit);
}

void MsgDispatcher::processGetAlgorithmParamCmd()
{
    COUT << "-----processGetSensorParamCmd-----" << endl;
    for (int i = 1; i <= 4; i++)
    {
        MqttPublishUnit reportUnit = {};
        reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
        reportUnit.content = DataFormater::FormatAlgorithmParamData(i);
        AppData::getInstance().addDataToDataSendQueue(reportUnit);
    }
}

void MsgDispatcher::processGetDtuParamCmd()
{
    COUT << "-----processGetDtuParamCmd-----" << endl;
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    reportUnit.content = DataFormater::FormatDtuParamData();
    AppData::getInstance().addDataToDataSendQueue(reportUnit);
}

void MsgDispatcher::processGetIni()
{
    COUT << "-----processGetINI-----" << endl;
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    reportUnit.content = DataFormater::FormatIniData();
    if (!reportUnit.content.empty())
    {
        AppData::getInstance().addDataToDataSendQueue(reportUnit);
    }
}

void MsgDispatcher::processGetJson()
{
    COUT << "-----processGetJSON-----" << endl;
    MqttPublishUnit reportUnit = {};
    reportUnit.topicIndex = MQTT_PUBLISH_TOPIC_INDEX_RESPONSE;
    reportUnit.content = DataFormater::FormatJsonData();
    if (!reportUnit.content.empty())
    {
        AppData::getInstance().addDataToDataSendQueue(reportUnit);
    }
}

void MsgDispatcher::processDfuCmd()
{
    COUT << "-----processDfuCmd-----" << endl;
    if (m_document.HasMember("reboot"))
    {
        int rebootVal = m_document["reboot"].GetInt();
        if (rebootVal > 0)
        {
            if (!GlobalFlag::getInstance().bOtaUpdating && !GlobalFlag::getInstance().bConfigUpdating && !GlobalFlag::getInstance().bDwinUpdating && !GlobalFlag::getInstance().bParamUpdating)
            {
                // 非升级指令的重启，正常重启系统
                COUT << "----reboot---system--" << endl;
                Utility::rebootSystem();
            }
            else if (GlobalFlag::getInstance().bOtaUpdating)
            {
                // 主进程OTA升级，替换主进程文件
                COUT << "----notify OTA-----" << endl;
                IPC::getInstance().send_message("OTA");
                GlobalFlag::getInstance().bOtaUpdating = false;
            }
            else if (GlobalFlag::getInstance().bParamUpdating)
            {
                // 参数文件升级 DO nothing
                COUT << "----参数文件升级 DO nothing-----" << endl;
                GlobalFlag::getInstance().bParamUpdating = false;
            }
            else
            {
                // 配置文件升级，迪文屏升级，重启主进程
                COUT << "----notify RESTART-----" << endl;
                IPC::getInstance().send_message("RESTART");
                GlobalFlag::getInstance().bDwinUpdating = false;
            }
        }
    }
    else if (m_document.HasMember("filename") && m_document.HasMember("host") && m_document.HasMember("path") && m_document.HasMember("port"))
    {
        string filename = m_document["filename"].GetString();
        string host = m_document["host"].GetString();
        string path = m_document["path"].GetString();
        int port = m_document["port"].GetInt();
        string fileUrl = host + ":" + to_string(port) + path + filename;
        if (isOtaPkg(filename)) // WeighBox升级
        {
            // HttpUtility::httpdownload(fileUrl, filename);
            Utility::writeMyConfig(DEFAULT_OTA_CONFIG_PATH, OTA_CONFIG_URL, fileUrl);
            // IPC::getInstance().send_message("OTA");
            GlobalFlag::getInstance().bOtaUpdating = true;
            Dwin::getInstance().switchPage(UI_DWIN_PAGE_UPGRADING);
        }
        else if (isDwinPkg(filename)) // Dwin升级
        {
            COUT << "----Dwin pkg-----" << fileUrl << endl;
            Utility::deleteDirectory(DEFAULT_DWIN_OTA_SAVE_PATH);
            Utility::createDirIfNotExist(DEFAULT_DWIN_OTA_SAVE_PATH);
            string pkgName = string(DEFAULT_DWIN_OTA_SAVE_PATH) + filename;
            HttpUtility::httpdownload(fileUrl, pkgName);
            // todo dwin update
            Dwin::getInstance().switchPage(UI_DWIN_PAGE_UPGRADING);
            GlobalFlag::getInstance().bDwinUpdating = true;
            int status = Utility::unzipFile(pkgName, DEFAULT_DWIN_OTA_SAVE_PATH); // 解压文件
            Dwin::getInstance().doDwinOTA(pkgName);
            Utility::deleteDirectory(DEFAULT_DWIN_OTA_SAVE_PATH);
        }
        else if (isConfigFile(filename)) // 配置文件升级
        {
            COUT << "----update  config-----" << fileUrl << endl;
            HttpUtility::httpdownload(fileUrl, filename);
            GlobalFlag::getInstance().bConfigUpdating = true;
            Dwin::getInstance().switchPage(UI_DWIN_PAGE_UPGRADING);
        }
        else if (isParamFile(filename))
        {
            COUT << "----update param-----" << fileUrl << endl;
            HttpUtility::httpdownload(fileUrl, filename);
            GlobalFlag::getInstance().bParamUpdating = true;
            Dwin::getInstance().switchPage(UI_DWIN_PAGE_UPGRADING);
            MdtuProv::getInstance().init();
            WeighAlgorithm::getInstance().init();
        }
    }
}

void MsgDispatcher::processCrtCmd()
{
    COUT << "-----processCrtCmd-----" << endl;
    string host = m_document["host"].GetString();
    int port = m_document["port"].GetInt();
    string path = m_document["path"].GetString();
    std::string logDate = m_document["date"].GetString();

    string uploadUrl = host + ":" + to_string(port) + path;
    COUT << "-----uploadUrl-----" << uploadUrl << endl;
    string deviceSN = Utility::getDeviceSN();

    // Do upload here
    std::string logFileName = DEFAULT_LOGFILE_PREFIX + logDate + ".log";
    std::string uploadFileName = std::string("wb_") + logFileName;
    Utility::copyFile(logFileName, uploadFileName);
    COUT << "-----uploadFileName-----" << uploadFileName << endl;
    usleep(1000000);
    if (Utility::fileExists(uploadFileName))
    {
        HttpUtility::httpUploadFile(uploadUrl, uploadFileName, uploadFileName, deviceSN);
    }
    else
    {
        COUT << "File not exist" << endl;
    }
    Utility::removeFile(uploadFileName);
}
