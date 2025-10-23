#include "GlobalFlag.h"
#include "DeviceConfig.h"
#include "Base.h"
#include "Utility.h"
#include "DataDef.h"

#define MQTT_REPORT_INTERVAL 2

GlobalFlag &GlobalFlag::getInstance()
{
    static GlobalFlag instance;
    return instance;
}

GlobalFlag::GlobalFlag()
{
    publishinterval = MQTT_REPORT_INTERVAL;
    bDataCanReport = false;
    bConfigUpdating = false;
    bOtaUpdating = false;
    bDwinUpdating = false;
    bParamUpdating = false;
    sensorProtocol = 0;
}

void GlobalFlag::init()
{
    bGpsDataLogOn = DeviceConfig::getInstance().get_value("dtu", "gpsDataLogOn") == "1" ? true : false;
    bSensorDataLogOn = DeviceConfig::getInstance().get_value("dtu", "sensorDataLogOn") == "1" ? true : false;
    bScreenDataLogOn = DeviceConfig::getInstance().get_value("dtu", "screenDataLogOn") == "1" ? true : false;
    bMqttDataLogOn = DeviceConfig::getInstance().get_value("dtu", "mqttDataLogOn") == "1" ? true : false;

    std::string strComputeMode = "";
    int computeMode = 0;
    if (!Utility::readMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_EDGE, strComputeMode))
    {
        computeMode = std::stoi(DeviceConfig::getInstance().get_value("dtu", "edgecomputing"));
    }
    else
    {
        computeMode = std::stoi(strComputeMode);
    }
    bEdgeComputeMode = computeMode > 0 ? true : false;
    bReportFullData = false;
    if (computeMode == COMPUTE_MODE_EDGE_ALL) // COMPUTE_MODE_EDGE_ALL 上报全数据
    {
        bReportFullData = true;
    }

    bCan = DeviceConfig::getInstance().get_value("dtu", "can") == "1" ? true : false;
    bCompresReport = DeviceConfig::getInstance().get_value("dtu", "compress") == "1" ? true : false;
    winstep = std::stoi(DeviceConfig::getInstance().get_value("dtu", "winstep"));
    publishinterval = std::stoi(DeviceConfig::getInstance().get_value("dtu", "publishinterval"));

    std::string strOffline = "";
    if (!Utility::readMyConfig(DEFAULT_CUSTOM_CONFIG_PATH, CUSTOM_CONFIG_OFFLINE, strOffline))
    {
        bOfflineMode = DeviceConfig::getInstance().get_value("dtu", "offline") == "1" ? true : false;
    }
    else
    {
        bOfflineMode = Utility::stringToBool(strOffline);
    }

    sensorType = DeviceConfig::getInstance().get_value("dtu", "SensorType");
    if (sensorType == DEFAULT_DTU_GC31_TYPE)
    {
        sensorProtocol = _MSENSOR_PROTOCOL_GC31;
    }
    else if (sensorType == DEFAULT_DTU_TDA04D_TYPE)
    {
        sensorProtocol = _WSENSOR_PROTOCL_4CHWEIGHT;
    }
    else if (sensorType == DEFAULT_DTU_ZY4701_TYPE)
    {
        sensorProtocol = _WSENSOR_PROTOCL_ZY4701;
    }
    else
    {
        sensorProtocol = _MSENSOR_PROTOCOL_MSENSOR;
    }

    COUT << "bGpsDataLogOn=" << bGpsDataLogOn << " bSensorDataLogOn=" << bSensorDataLogOn << " bScreenDataLogOn=" << bScreenDataLogOn << " bMqttDataLogOn=" << bMqttDataLogOn << endl;
    COUT << "bEdgeComputeMode=" << bEdgeComputeMode << " bReportFullData=" << bReportFullData << " bOfflineMode=" << bOfflineMode << " bCan=" << bCan << " bCompresReport=" << bCompresReport << endl;
    COUT << "winstep=" << winstep << " publishinterval=" << publishinterval << endl;
    COUT << "sensorType=" << sensorType << " sensorProtocol=" << sensorProtocol << endl;
    COUT << "bConfigUpdating=" << bConfigUpdating << " bOtaUpdating=" << bOtaUpdating << " bDwinUpdating=" << bDwinUpdating << "bParamUpdating=" << bParamUpdating << endl;
#ifdef EXCEPTION_REPORT
    exceptionCode = EXCEPTION_SERVER_COM;
    expSensors = 0; // 传感器数据异常时的传感器位置
    expGC31s = 0;   // GC31通信异常时，GC31的轴数
#endif
}

bool GlobalFlag::isUpdating()
{
    return (bConfigUpdating || bOtaUpdating || bDwinUpdating);
}
