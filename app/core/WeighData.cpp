#include "WeighData.h"
#include "DeviceConfig.h"
#include <iostream>
#include <iomanip>
#include "Utility.h"
#include "DataDef.h"
#include "MdtuProv.h"

int getSensorIDByKey(std::string sensorKey)
{
    // 假设sensorKey的格式为“sensorxx”，其中xx为传感器ID
    std::string idStr = sensorKey.substr(6); // 截取字符串中的数字部分
    int sensorID = std::stoi(idStr);         // 将数字部分转换为整数
    return sensorID;
}

WeighData &WeighData::getInstance()
{
    static WeighData instance;
    return instance;
}

WeighData::WeighData() : initialized(false), currentTareWeight(0)
{
    bOverload = false;
    currentReportData = "";
}

void WeighData::init()
{
    if (initialized)
        return;
    initialized = true;
    // 从weigh_info.ini中获取皮重，如果没有，就从mdtuprov.json中取得
    string strTareWeight = "";
    if (!Utility::readMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_TAREWEIGHT, strTareWeight))
    {
        MdtuUnit dtu = MdtuProv::getInstance().getMdtuUnit(DTU_PROV_NAME_TAREWEIGHT);
        currentTareWeight = static_cast<float>(dtu.iValue) / 1000;
        // printf("-------WeighData-----%f---\n", currentTareWeight);
    }
    else
    {
        currentTareWeight = std::stof(strTareWeight);
    }

    cout << "----------" << endl;
    vector<string> deviceKeyVec = DeviceConfig::getInstance().get_key_list("name");
    for (std::string sensorKey : deviceKeyVec)
    {
        SensorDataUnit dataUnit = {};
        dataUnit.bActive = (stoi(DeviceConfig::getInstance().get_value("active", sensorKey)) > 0) ? true : false;
        if (dataUnit.bActive)
        {
            dataUnit.protocal = stoi(DeviceConfig::getInstance().get_value("protocol", sensorKey));
            dataUnit.portID = stoi(DeviceConfig::getInstance().get_value("portid", sensorKey));
            dataUnit.sensorID = getSensorIDByKey(sensorKey);
            dataUnit.sensorKey = sensorKey;
            dataUnit.sensorName = DeviceConfig::getInstance().get_value("name", sensorKey);
            dataUnit.sensorAddr = static_cast<unsigned char>(std::stoul(DeviceConfig::getInstance().get_value("nodeid", sensorKey)));
            dataUnit.bSettingUpdated = true;
            sensor_num++;
            currentWeighData.sensorDataMap.insert(std::make_pair(dataUnit.sensorAddr, dataUnit));
        }
        usleep(100000);
    }

    for (auto it = WeighData::getInstance().currentWeighData.sensorDataMap.begin(); it != WeighData::getInstance().currentWeighData.sensorDataMap.end(); ++it)
    {
        SensorDataUnit *pData = &(it->second);
        if (pData->bActive)
        {
            if (pData->protocal == _MSENSOR_PROTOCOL_GC31)
            {
                GC31::getInstance().getDeviceSN(pData->sensorAddr);
                if (pData->bSettingUpdated)
                {
                    GC31::getInstance().getDeviceSetting(pData->sensorAddr);
                    // pData->bSettingUpdated = false;
                }
                GC31::getInstance().getVersion(pData->sensorAddr);
            }
            else if (pData->protocal == _WSENSOR_PROTOCL_4CHWEIGHT)
            {
                // 台秤传感器，不需要做什么
                cout << "---台秤传感器TDA04d---" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pData->sensorAddr) << "-----" << pData->sensorKey << "-----" << pData->sensorName << endl;
            }
            else if (pData->protocal == _WSENSOR_PROTOCL_ZY4701)
            {
                // 变送器ZY4701，不需要做什么
                if (pData->bSettingUpdated)
                {
                    ZY4701::getInstance().getDeviceSN(pData->sensorAddr, pData->sensorSN, pData->settings.version);
                    bool bSuccess = false;
                    for (int i = 0; i < SENSOR_MAX_NUM; i++)
                    {
                        bSuccess = ZY4701::getInstance().getDeviceSetting(pData->sensorAddr, i, pData->settings.gain[i], pData->settings.reverseFlag[i], pData->settings.filter[i], pData->settings.autoCalib[i]);
                    }
                    if (bSuccess)
                    {
                        pData->bSettingUpdated = false;
                    }
                }
                cout << "---变送器ZY4701---" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pData->sensorAddr) << "-----" << pData->sensorKey << "-----" << pData->sensorName << endl;
            }
        }
    }
    MdtuUnit loadcapDtu = MdtuProv::getInstance().getMdtuUnit(DTU_PROV_NAME_LOADCAP);
    loadCap = static_cast<float>(loadcapDtu.iValue) / 1000;
    // printf("------loadCap---%f--\n", loadCap);
    COUT << "loadCap=" << loadCap << endl;
}

void WeighData::notifySensorSettingChanged(uint8_t sensorKey, bool bUpdated)
{
    COUT << "---------notifySensorSettingChanged----------" << endl;
    currentWeighData.sensorDataMap[sensorKey].bSettingUpdated = bUpdated;
}

void WeighData::saveReportData(std::string data)
{
    std::lock_guard<std::mutex> lock(data_mutex);
    currentReportData = data;
}

std::string WeighData::getReportData()
{
    std::lock_guard<std::mutex> lock(data_mutex);
    return currentReportData;
}
