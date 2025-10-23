// appdata.h
#ifndef GLOBALFLAG_H
#define GLOBALFLAG_H

#include <queue>
#include <array>
#include <mutex>
#include "DataDef.h"

class GlobalFlag
{

private:
    GlobalFlag();                                       // private constructor to prevent instantiation
    GlobalFlag(const GlobalFlag &) = delete;            // delete the copy constructor
    GlobalFlag &operator=(const GlobalFlag &) = delete; // delete the assignment operator

public:
    static GlobalFlag &getInstance();
    void init();
    bool isUpdating();
    bool bGpsDataLogOn;
    bool bSensorDataLogOn;
    bool bScreenDataLogOn;
    bool bMqttDataLogOn;

    bool bEdgeComputeMode;
    bool bReportFullData;
    bool bOfflineMode;
    bool bDataCanReport;
    bool bCompresReport;
    int winstep;
    bool bCan;
    int publishinterval;

    bool bConfigUpdating;
    bool bOtaUpdating;
    bool bDwinUpdating;
    bool bParamUpdating;

    int sensorProtocol;
    std::string sensorType;

#ifdef EXCEPTION_REPORT
    uint8_t exceptionCode;
    uint8_t expSensors; // 传感器数据异常时的传感器位置
    uint8_t expGC31s;   // GC31通信异常时，GC31的轴数
#endif
};

#endif // GLOBALFLAG_H
